#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#define TCP_STREAM SOCK_STREAM
#define UDP_STREAM SOCK_DGRAM

void die(char *s) {
  fprintf(stderr, "%s\n", s);
  exit(1);
}
int connect_server(char *arg_ip, char *arg_port) {
  char *to_addr = arg_ip;
  char **endptr = NULL;
  int to_port = strtol(arg_port, endptr, 0);
  if (endptr != NULL) die("port is not valid");
  int s = socket(PF_INET, UDP_STREAM, 0);
  if (s == 1) die("socket error");
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  int ret = inet_aton(to_addr, &addr.sin_addr);
  if (ret ==-1) die("ip address is not valid");
  addr.sin_port = htons(to_port);
  ret = connect(s, (struct sockaddr*)&addr, sizeof(addr));
  if (ret == -1) {
    die("connect failed");
  }
  fprintf(stderr, "[info] connection success\n");

  return s;
}

#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <opencv2/face/facemark.hpp>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <utility>
#include <tuple>
#include <string>
#include <algorithm>
#define HEIGHT 480
#define WIDTH 640

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

Mat cam_matrix = (Mat_<double>(3,3) << 6.5308391993466671e2, 0, 3.195e2, 0, 6.5308391993466671e2, 2.395e2, 0, 0, 1);
vector<double> dist_coeffs = {7.0834633684407095e-2, 6.9140193737175351e-2, 0, 0, -1.3073460323689292};

vector<Point3f> object_pts = {Point3f(6.825897, 6.760612, 4.402142), Point3f(1.330353, 7.122144, 6.903745), Point3f(-1.330353, 7.122144, 6.903745), Point3f(-6.825897, 6.760612, 4.402142), Point3f(5.311432, 5.485328, 3.987654), Point3f(1.789930, 5.393625, 4.413414), Point3f(-1.789930, 5.393625, 4.413414), Point3f(-5.311432, 5.485328, 3.987654), Point3f(2.005628, 1.409845, 6.165652), Point3f(-2.005628, 1.409845, 6.165652), Point3f(2.774015, -2.080775, 5.5048531), Point3f(-2.774015, -2.080775, 5.5048531), Point3f(0, -3.116408, 6.097667), Point3f(0, -7.415691, 4.070434)};


vector<double> get_head_pose(vector<Point2f> shape) {
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

int main() {
  VideoCapture cap(0);
  assert(cap.isOpened());
  cap.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);

  Mat img, gray;
  CascadeClassifier faceDetector("haarcascade_frontalface_alt2.xml");
  Ptr<Facemark> facemark = FacemarkLBF::create();
  facemark->loadModel("lbfmodel.yaml");
  
  vector<cv::Rect> faces;
  vector<vector<Point2f>> landmarks;
  vector<pair<Kalman, Kalman>> kalmans(68, make_pair(Kalman(0.5, 2), Kalman(0.5, 2)));
  vector<Kalman> face_kalmans(3, Kalman(0.5, 1));

  int s = connect_server("127.0.0.1", "55555");

  while (cap.read(img)) {
    cvtColor(img, gray, COLOR_BGR2GRAY);
    faceDetector.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(20, 20));
    for (int i = 0; i < faces.size(); i++) {
      cv::rectangle(img, cv::Point(faces[i].x, faces[i].y), cv::Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), cv::Scalar(255, 0, 0), 3, CV_AA);
    }

    bool success = facemark->fit(img, faces, landmarks);
    if (success) {
      // landmarks.size()>1なら一番大きな顔を対象に実行する
      int target = 0;
      for(int i = 0; i < landmarks.size(); i++) {
        if (faces[i].width * faces[i].height > faces[target].width * faces[target].height) {
          target = i;
        }
      }
      for(int j = 0; j < landmarks[target].size(); j++) {
        double x = kalmans[j].first.guess(landmarks[target][j].x);
        double y = kalmans[j].second.guess(landmarks[target][j].y);
        // cv::putText(img, to_string(j), cv::Point2f(x, y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255, FILLED), 2, CV_AA); 
        cv::circle(img, Point2f(x, y), 3, cv::Scalar(0, 0, 255, FILLED));
      }
      vector<double> face_pose = get_head_pose(landmarks[target]);
      swap(face_pose[1], face_pose[2]);
      for(int j = 0; j < 3; j++)  face_pose[j] = face_kalmans[j].guess(face_pose[j]);
      char buf[18];
      sprintf(buf, "%.3f %.3f %.3f ", face_pose[0], face_pose[1], face_pose[2]);
      fprintf(stderr, "%s\n", buf);
      int m = send(s, buf, sizeof(buf), 0);
      if (m != sizeof(buf)) die("failed to send data");
    }

    cv::imshow("tvphone", img);

    if (cv::waitKey(1) == 'q')  break;
    faces.clear();
  }

  cv::destroyAllWindows();
  return 0;

}

