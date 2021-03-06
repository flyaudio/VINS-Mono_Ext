/*
    test camera model, especially distortion mapping 
 */
#include <cstdio>
#include <iostream>
#include <string>
#include <eigen3/Eigen/Dense>
 using namespace std; 
 class Cam
{
public:
    Cam()
    {
	fx = 190.978; cx = 254.932;
	fy = 190.973; cy = 256.897;
	k1_ = 0.00348239; k2_ = 0.000715035; k3_ = - 0.00205324; k4_ = 0.000202937; 
    };
    void distortEquidist(const Eigen::Vector2d& in, Eigen::Vector2d& out, Eigen::Matrix2d& J) const; 
    double fx;
    double fy;
    double cx;
    double cy; 
    double k1_;
    double k2_;
    double k3_;
    double k4_; 
};
 void test_cam_model(); 
bool pixelToBearing(Cam& cam, const Eigen::Vector2d& c,Eigen::Vector3d& vec);
 int main(int argc, char* argv[])
{
    test_cam_model();
    return 0; 
}
 void test_cam_model()
{
    Eigen::Vector2d a(25.5, 67.5); // (419, 103); // 15.5, 57.5
    Eigen::Vector3d b;
    Cam cam; 
    cout <<"input a = "<<endl<<a<<endl;
     if(pixelToBearing(cam, a, b))
    {
	cout <<"succeed to converge! a: "<<a<<" b: "<<b<<endl; 
    }else
    {
	cout <<"failed to converge!"<<endl; 
    }
    return ;
}
 bool pixelToBearing(Cam& cam, const Eigen::Vector2d& c,Eigen::Vector3d& vec) 
{
    Eigen::Vector2d y;
    double fx = cam.fx; 
    double fy = cam.fy; 
    double cx = cam.cx;
    double cy = cam.cy; 
    
    y(0) = (static_cast<double>(c.x()) - cx) / fx;
    y(1) = (static_cast<double>(c.y()) - cy) / fy;
     // Undistort by optimizing
    const int max_iter = 5;
    const double tolerance = 1e-10; // 1e-10
    Eigen::Vector2d ybar = y; // current guess (undistorted)
    Eigen::Matrix2d J;
    Eigen::Vector2d y_tmp; // current guess (distorted)
    Eigen::Vector2d e;
    Eigen::Vector2d du;
    bool success = false;
    for (int i = 0; i < max_iter; i++) {
	cam.distortEquidist(ybar, y_tmp, J); 
	e = y - y_tmp;
	du = (J.transpose() * J).inverse() * J.transpose() * e;
	ybar += du;
	if (e.dot(e) <= tolerance){
	    success = true;
	    break;
	}else{
	     std::cout<<" y = "<<y<<" y_tmp = "<<y_tmp<<" e = "<<e<<std::endl;
	     std::cout<<"e.dot(e) = "<<e.dot(e)<<" > tolerance: "<<tolerance<<std::endl;
	     std::cout<<"du = "<<du(0)<<", "<<du(1)<<" ybar = "<<ybar(0)<<", "<<ybar(1)<<std::endl;
	}
	if(du!=du)
	{
	    std::cout<<"du = "<<du<<" break!"<<std::endl;
	    break;
	}
    }
    if(success){
	y = ybar;
	vec = Eigen::Vector3d(y(0),y(1),1.0).normalized();
    }
    return success;
}
 void Cam::distortEquidist(const Eigen::Vector2d& in, Eigen::Vector2d& out, Eigen::Matrix2d& J) const
{
    const double x2 = in(0) * in(0);
    const double y2 = in(1) * in(1);
    const double r = std::sqrt(x2 + y2);
     if(r < 1e-8){
	out(0) = in(0);
	out(1) = in(1);
	J.setIdentity();
	return;
    }
     const double r_x = 1/r*in(0);
    const double r_y = 1/r*in(1);
     const double th = atan(r); // 1/(r^2 + 1)
    const double th_r = 1/(r*r+1);
    const double th2 = th*th;
    const double th4 = th2*th2;
    const double th6 = th2*th4;
    const double th8 = th2*th6;
    const double thd = th * (1.0 + k1_ * th2 + k2_ * th4 + k3_ * th6 + k4_ * th8);
    const double thd_th = 1.0 + 3 * k1_ * th2 + 5* k2_ * th4 + 7 * k3_ * th6 + 9 * k4_ * th8;
    const double s = thd/r;
    const double s_r = thd_th*th_r/r - thd/(r*r);
     out(0) = in(0) * s;
    out(1) = in(1) * s;
     J(0,0) = s + in(0)*s_r*r_x;
    J(0,1) = in(0)*s_r*r_y;
    J(1,0) = in(1)*s_r*r_x;
    J(1,1) = s + in(1)*s_r*r_y;
}
