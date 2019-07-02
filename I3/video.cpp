#include "video.hpp"
#include "connect.hpp"

#define PACKET_VIDEO_SIZE 18
#define HEIGHT 480
#define WIDTH 640
#define VIDEO_DEBUG

using namespace std;
using namespace cv;
using namespace cv::face;

// opencvそのままやると特徴点がぶれまくるのでカルマンフィルターでごまかしておく
class Kalman {
  private:
    const bool DEBUG = false;
    int ctr;
    double Q, P_k_minus_1, R, K;
    double xhat_k_minus_1, xhat_k, Phat_k;
  public:
    Kalman(double procvar, double measvar) {
      ctr = 0;
      Q = procvar;
      P_k_minus_1 = 0;
      R = measvar;
    }

    double guess(double input) {
      if (ctr == 0) {
        xhat_k_minus_1 = input;
        K = 1.0;
        ctr++;
        return input;
      }
      else {
        xhat_k = xhat_k_minus_1;
        Phat_k = P_k_minus_1 + Q;
        K = Phat_k / (Phat_k + R);
        double estimate = xhat_k + K * (input - xhat_k);
        xhat_k_minus_1 = estimate;
        P_k_minus_1 = (1 - K) * Phat_k;

        if (DEBUG) {
          fprintf(stderr, "Kalman: Input%lf Estimate%lf ErrorVar%lf KalmanGain%lf\n", input, estimate, P_k_minus_1, K);
        }
        return estimate;
      }
    }
};

vector<double> get_head_pose(vector<Point2f> shape) {
  static Mat cam_matrix = (Mat_<double>(3,3) << 6.5308391993466671e2, 0, 3.195e2, 0, 6.5308391993466671e2, 2.395e2, 0, 0, 1);
  static vector<double> dist_coeffs = {7.0834633684407095e-2, 6.9140193737175351e-2, 0, 0, -1.3073460323689292};
  static vector<Point3f> object_pts = {Point3f(6.825897, 6.760612, 4.402142), Point3f(1.330353, 7.122144, 6.903745), Point3f(-1.330353, 7.122144, 6.903745), Point3f(-6.825897, 6.760612, 4.402142), Point3f(5.311432, 5.485328, 3.987654), Point3f(1.789930, 5.393625, 4.413414), Point3f(-1.789930, 5.393625, 4.413414), Point3f(-5.311432, 5.485328, 3.987654), Point3f(2.005628, 1.409845, 6.165652), Point3f(-2.005628, 1.409845, 6.165652), Point3f(2.774015, -2.080775, 5.5048531), Point3f(-2.774015, -2.080775, 5.5048531), Point3f(0, -3.116408, 6.097667), Point3f(0, -7.415691, 4.070434)};

  vector<Point2f> image_pts = {shape[17], shape[21], shape[22], shape[26], shape[36], shape[39], shape[42], shape[45], shape[31], shape[35], shape[48], shape[54], shape[57], shape[8]};
  Mat rotation_vec, translation_vec;
  bool ok = solvePnP(object_pts, image_pts, cam_matrix, dist_coeffs, rotation_vec, translation_vec); 
  Mat rotation_mat;
  Rodrigues(rotation_vec, rotation_mat);
  Mat pose_mat;
  hconcat(rotation_mat, translation_vec, pose_mat);
  Mat cam_out_mat, rot_out_mat, trans_out_vect, xrot_out, yrot_out, zrot_out;
  vector<double> euler_angles;
  decomposeProjectionMatrix(pose_mat, cam_out_mat, rot_out_mat, trans_out_vect, xrot_out, yrot_out, zrot_out, euler_angles);
  return euler_angles;
}

void send_video(int s) {
  VideoCapture cap(0);
  if (!cap.isOpened()) {
    die("failed to open camera");
  } 
  
  cap.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);

  Mat img, gray;
  CascadeClassifier faceDetector("test/haarcascade_frontalface_alt2.xml");
  Ptr<Facemark> facemark = FacemarkLBF::create();
  facemark->loadModel("test/lbfmodel.yaml");
  
  vector<Rect> faces;
  vector<vector<Point2f>> landmarks;
  vector<pair<Kalman, Kalman>> kalmans(68, make_pair(Kalman(0.5, 2), Kalman(0.5, 2)));
  vector<Kalman> face_kalmans(3, Kalman(0.5, 1));

  char buf[PACKET_VIDEO_SIZE];

  while (cap.read(img)) {
    cvtColor(img, gray, COLOR_BGR2GRAY);

    faceDetector.detectMultiScale(gray, faces, 1.1, 3, 0, Size(20, 20));
    #ifdef VIDEO_DEBUG
    for (int i = 0; i < faces.size(); i++) {
      rectangle(img, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(255, 0, 0), 3, CV_AA);
    }
    #endif

    bool success = facemark->fit(img, faces, landmarks);
    if (success) {
      // 一番大きな顔を対象とする
      int target = 0;
      for (int i = 0; i < landmarks.size(); i++) {
        if (faces[i].width * faces[i].height > faces[target].width * faces[target].height) {
          target = i;
        }
      }
      for (int i = 0; i < landmarks[target].size(); i++) {
        double x = kalmans[i].first.guess(landmarks[target][i].x);
        double y = kalmans[i].second.guess(landmarks[target][i].y);
        #ifdef VIDEO_DEBUG
        cv::circle(img, Point2f(x, y), 3, cv::Scalar(0, 0, 255, FILLED));
        #endif VIDEO_DEBUG
      }

      vector<double> face_pose = get_head_pose(landmarks[target]);
      swap(face_pose[1], face_pose[2]);
      for (int i = 0; i < 3; i++) face_pose[i] = face_kalmans[i].guess(face_pose[i]);
      sprintf(buf, "%.3f %.3f %.3f ", face_pose[0], face_pose[1], face_pose[2]);
      fprintf(stderr, "%s\n", buf);
      
      int m = send(s, buf, sizeof(buf), 0);
      if (m != sizeof(buf)) die("failed to send data");
    }
  }
}

// sから受け取ってtに流す
void recv_video(int s, int t) {
  char buf[PACKET_VIDEO_SIZE];
  while (true) {
    int m = recv(s, buf, PACKET_VIDEO_SIZE, 0);
    if (m == 0) break;

    int m2 = send(t, buf, sizeof(buf), 0);
    if (m != m2)  die("failed to send data");
  }
}

void send_recv_video(int s) {
  try {
    thread sender(send_video, s);
    int unity = connect_server_udp("127.0.0.1", "55555");
    thread recver(recv_video, s, unity);

    sender.join();
    recver.join();
    close(unity);
  }
  catch(cv::Exception &e) {
    cerr << e.what() << endl;
  }
  close(s);
}
