#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <math.h>
#include<iostream>
#include<vector>

const float PI = 3.141592653589793238462643383279502884;
using namespace cv;
using namespace std;
struct point 
{
	float x;
	float y;
};
void matrixmul(const Mat& a, const Mat& b, Mat& result) 
{
	for (int i = 0; i < a.rows; i++) 
	{
		for (int j = 0; j < b.cols; j++) 
		{
			result.at<float>(i, j) = 0;
			for (int z = 0; z < a.cols; z++) 
			{
				result.at<float>(i, j) += a.at<float>(i, z) * b.at<float>(z, j);
			}
		}
	}
}
Mat matrixmul(const Mat& a, const Mat& b) 
{
  Mat result(a.rows, b.cols, a.type());
	for (int i = 0; i < a.rows; i++) 
	{
		for (int j = 0; j < b.cols; j++) 
		{
			result.at<float>(i, j) = 0;
			for (int z = 0; z < a.cols; z++) 
			{
				result.at<float>(i, j) += a.at<float>(i, z) * b.at<float>(z, j);
			}
		}
	}
	return result;
}

/*
Lớp biểu diễn pháp biến đổi affine
*/
class AffineTransform
{
	Mat _reverse;// ma trận 3x3 biểu diễn affine nghịch đảo
	Mat _matrixTransform;//ma trận 3x3 biểu diễn phép biến đổi affine
	Mat _tmp;
	Mat _tmp_point; //ma trận 3x1 biểu diễn điểm
public:
	void Translate(float dx, float dy)// xây dựng matrix transform cho phép tịnh tiến theo vector (dx,dy)
	{
		swap(dx, dy);//do tính toán bằng tọa độ ma trận nên phải đảo lại
		Mat temp = Mat(3, 3, CV_32FC1);
		Mat temp1 = Mat(3, 3, CV_32FC1);
		if (temp.empty() || temp1.empty())return;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) 
			{
				temp.at<float>(i, j) = 0;
				temp1.at<float>(i, j) = 0;
			}
		}
		for (int i = 0; i < 3; i++) 
		{
			temp.at<float>(i, i) = 1;
			temp1.at<float>(i, i) = 1;
		}
		temp.at<float>(0, 2) = dx;
		temp.at<float>(1, 2) = dy;
		temp1.at<float>(0, 2) = -dx;// tăng 1 đoạn x thì affine nghịch phải giảm 1 đoạn x
		temp1.at<float>(1, 2) = -dy;// tăng 1 đoạn y thì phải giảm 1 đoạn dy
		matrixmul(temp, _matrixTransform, _tmp);
		_tmp.copyTo(_matrixTransform);
		matrixmul(_reverse, temp1, _tmp);// đổi thứ tự, ví dụ affine thuận là scale rồi xoay, thì affine nghịch phải xoay rồi mới scale ngược.
		_tmp.copyTo(_reverse);
	}
	void Rotate(float angle)//xây dựng matrix transform cho phép xoay 1 góc angle đơn vị là độ
	{
		angle = angle * PI / 180;
		Mat temp = Mat(3, 3, CV_32FC1);
		Mat temp1 = Mat(3, 3, CV_32FC1);
		if (temp.empty() || temp1.empty())return;
		for (int i = 0; i < 3; i++) 
		{
			for (int j = 0; j < 3; j++) 
			{
				temp.at<float>(i, j) = 0;
				temp1.at<float>(i, j) = 0;
			}
		}
		temp.at<float>(0, 0) = cos(angle);
		temp.at<float>(0, 1) = -sin(angle);
		temp.at<float>(1, 0) = sin(angle);
		temp.at<float>(1, 1) = cos(angle);
		temp.at<float>(2, 2) = 1;


		temp1.at<float>(0, 0) = cos(-angle);// ngược lại với affine thuận
		temp1.at<float>(0, 1) = -sin(-angle);
		temp1.at<float>(1, 0) = sin(-angle);
		temp1.at<float>(1, 1) = cos(-angle);
		temp1.at<float>(2, 2) = 1;

		matrixmul(temp, _matrixTransform, _tmp);
		_tmp.copyTo(_matrixTransform);
		matrixmul(_reverse, temp1, _tmp);// đổi thứ tự, ví dụ affine thuận là scale rồi xoay, thì affine nghịch phải xoay rồi mới scale ngược.
		_tmp.copyTo(_reverse);

	}
	void Scale(float sx, float sy) //xây dựng matrix transform cho phép tỉ lệ theo hệ số
	{
		swap(sx, sy); // do ở bên dưới tính toán bằng tọa độ ma trận nên sx với sy bị ngược so với thực tế
		Mat temp = Mat(3, 3, CV_32FC1);
		Mat temp1 = Mat(3, 3, CV_32FC1);
		if (temp.empty() || temp1.empty())return;
		for (int i = 0; i < 3; i++) 
		{
			for (int j = 0; j < 3; j++) 
			{
				temp.at<float>(i, j) = 0;
				temp1.at<float>(i, j) = 0;
			}
		}
		temp.at<float>(0, 0) = sx;
		temp.at<float>(1, 1) = sy;
		temp.at<float>(2, 2) = 1;

		temp1.at<float>(0, 0) = 1.0 / sx;// scale 1 đoạn sx thì scale ngược 1 đoạn 1/sx
		temp1.at<float>(1, 1) = 1.0 / sy;//tương tự
		temp1.at<float>(2, 2) = 1;

		matrixmul(temp, _matrixTransform, _tmp);
		_tmp.copyTo(_matrixTransform);
		matrixmul(_reverse, temp1, _tmp);// đổi thứ tự, ví dụ affine thuận là scale rồi xoay, thì affine nghịch phải xoay rồi mới scale ngược.
		_tmp.copyTo(_reverse);

	}
	void TransformPoint(float& x, float& y)//transform 1 điểm (x,y) theo matrix transform đã có
	{
		Mat temp = Mat(3, 1, CV_32FC1);
		if (temp.empty())return;
		temp.at<float>(0, 0) = x;
		temp.at<float>(1, 0) = y;
		temp.at<float>(2, 0) = 1;
		matrixmul(_matrixTransform, temp, _tmp_point);// nhân affine thuận 3x3 với ma trận 3x1 để ra x , y mới
		x = _tmp_point.at<float>(0, 0);
		y = _tmp_point.at<float>(1, 0);
	}

	void InversePoint(float& x, float& y) 
	{
		Mat temp = Mat(3, 1, CV_32FC1);
		if (temp.empty())return;
		temp.at<float>(0, 0) = x;
		temp.at<float>(1, 0) = y;
		temp.at<float>(2, 0) = 1;
		matrixmul(_reverse, temp, _tmp_point);
		x = _tmp_point.at<float>(0, 0);// tương tự như trên nhưng đây là affine nghịch
		y = _tmp_point.at<float>(1, 0);

	}

	AffineTransform() 
	{
		_matrixTransform = Mat(3, 3, CV_32FC1);//khởi tạo ma trận đơn vị cho affine nghịch đảo
		_reverse = Mat(3, 3, CV_32FC1);
		_tmp = Mat(3, 3, CV_32FC1);
		_tmp_point = Mat(3, 1, CV_32FC1);
		for (int i = 0; i < 3; i++) 
		{
			for (int j = 0; j < 3; j++) 
			{
				_matrixTransform.at<float>(i, j) = 0;
				_reverse.at<float>(i, j) = 0;
			}
		}

		for (int i = 0; i < 3; i++) 
		{
			_matrixTransform.at<float>(i, i) = 1;
			_reverse.at<float>(i, i) = 1;
		}
	}
	//_matrixTransform = Mat(3, 3, CV_32FC1);
	~AffineTransform() {}
};

class PixelInterpolate
{

public:

	/*
	Hàm tính giá trị màu của ảnh kết quả từ nội suy màu trong ảnh gốc
	Tham số
		- (tx,ty): tọa độ thực của ảnh gốc sau khi thực hiện phép biến đổi affine
		- pSrc: con trỏ ảnh gốc
		- srcWidthStep: widthstep của ảnh gốc
		- nChannels: số kênh màu của ảnh gốc
	Trả về
		- Giá trị màu được nội suy
	*/
	virtual void Interpolate(uchar* pDest,
		float tx, float ty, uchar* pSrc, int srcWidthStep, int nChannels) = 0;
	virtual void setAffineMatrix(AffineTransform* matrix) = 0;
	PixelInterpolate() {}
	~PixelInterpolate() {}
};

/*
Lớp nội suy màu theo phương pháp láng giềng gần nhất
*/
class NearestNeighborInterpolate : public PixelInterpolate
{
	AffineTransform* tf;// cần phép affine gốc để có matrận nghịch đảo, nếu không thì sẽ không có đủ thông tin nội suy
	Mat scr; // cần giới hạn chiều cao chiều dài ảnh
public:
	void Interpolate(uchar* pDest, float tx, float ty, uchar* pSrc, int srcWidthStep, int nChannels) 
	{
		tf->InversePoint(tx, ty); // đặt tx=y và ty = x, vì lí do xét theo ma trận.
		tx = round(tx);
		ty = round(ty);
		if ((tx < 0 or tx >= scr.rows) or (ty < 0 or ty >= scr.cols)) 
		{
  		for (int i = 0; i < nChannels; i++) 
  		{
  			pDest[i] = 0;
  		}
		}
		int x = (int)tx;
		int y = (int)ty;
		pSrc += x * srcWidthStep;
		pSrc += y * nChannels;
		for (int i = 0; i < nChannels; i++) 
		{
			pDest[i] = pSrc[i];
		}

	}
	NearestNeighborInterpolate(AffineTransform* affine1, Mat source) 
	{
		this->tf = affine1;
		this->scr = source;
	}
	void setAffineMatrix(AffineTransform* matrix) 
	{
		this->tf = matrix;
	}
	~NearestNeighborInterpolate() {}
};

/*
Lớp thực hiện phép biến đổi hình học trên ảnh
*/

class GeometricTransformer
{
public:
	/*
	Hàm biến đổi ảnh theo 1 phép biến đổi affine đã có
	Tham số
	 - beforeImage: ảnh gốc trước khi transform
	 - afterImage: ảnh sau khi thực hiện phép biến đổi affine
	 - transformer: phép biến đổi affine
	 - interpolator: biến chỉ định phương pháp nội suy màu
	Trả về:
	 - 0: Nếu ảnh input ko tồn tại hay ko thực hiện được phép biến đổi
	 - 1: Nếu biến đổi thành công
	*/
	int Transform(const Mat& beforeImage, Mat& afterImage, AffineTransform* transformer, PixelInterpolate* interpolator) // tương tự Ox là tọa độ rows (hướng dọc xuống), Oy là tọa độ columms(từ trái qua phải)
	{
		if (beforeImage.empty())return 0;
		float x = 0;
		float y = 0;
		point temp[4];
		//tìm tọa độ mới thông qua affine của tọa độ 4 góc ảnh của ảnh gốc
		transformer->TransformPoint(x, y);
		temp[0].x = x;
		temp[0].y = y;
		x = 0;
		y = beforeImage.cols - 1;
		transformer->TransformPoint(x, y);
		temp[1].x = x;
		temp[1].y = y;
		x = beforeImage.rows - 1;
		y = 0;
		transformer->TransformPoint(x, y);
		temp[2].x = x;
		temp[2].y = y;
		x = beforeImage.rows - 1;
		y = beforeImage.cols - 1;
		transformer->TransformPoint(x, y);
		temp[3].x = x;
		temp[3].y = y;
		float minx, maxx, miny, maxy;
		minx = maxx = temp[0].x;
		miny = maxy = temp[0].y;
		for (int i = 0; i < 4; i++) 
		{
			if (temp[i].x > maxx) maxx = temp[i].x;
			if (temp[i].y > maxy) maxy = temp[i].y;
			if (temp[i].x < minx) minx = temp[i].x;
			if (temp[i].y < miny) miny = temp[i].y;
		}//tìm giá trị lớn nhất nhỏ nhất của x và y để tính size ảnh mới
		float newheight = ceil(maxx - minx);
		float newwidth = ceil(maxy - miny);
		afterImage = Mat(newheight, newwidth, beforeImage.type());
		if (afterImage.empty())return 0;
		else 
		{
			uchar* pdataSource = (uchar*)beforeImage.data;
			uchar* pdataDes = (uchar*)afterImage.data;
			int widthStepSource = beforeImage.step[0];
			int widthStepDes = afterImage.step[0];
			int nChannelsSource = beforeImage.step[1];
			int nChannelsDes = afterImage.step[1];
			//cout << nChannelsDes << endl;
			for (int i = 0; i < afterImage.rows; i++) 
			{
				uchar* prowDes = pdataDes;
				for (int j = 0; j < afterImage.cols; j++)
				{
					interpolator->Interpolate(prowDes, i + minx, j + miny, pdataSource, widthStepSource, nChannelsSource);//duyệt ảnh đích, so vvới giá trị ảnh gốc để lấy giá trị điểm ảnh ra
					prowDes += nChannelsDes;
				}
				pdataDes += widthStepDes;
			}
			return 1;
		}
	}
	int Flip(
		const Mat& srcImage,
		Mat& dstImage,
		bool direction,
		PixelInterpolate* interpolator) 
	{
		if (srcImage.empty())
			return 0;
		Mat dest = srcImage.clone();

		int nChanel = srcImage.step[1];
		int nCols = srcImage.cols;
		int nRows = srcImage.rows;


		uchar* pDataSource = (uchar*)srcImage.data;
		uchar* pDataDes = (uchar*)dest.data;
		int widthStep = srcImage.step[0];
		int widthStepDes = dest.step[0];
		int nChanelDes = dest.step[1];
		//Lấy đối xứng qa Ox, tương đương với CmdArguments = 1
		if (direction == 0) {
			for (int i = 0; i < nRows; i++) 
			{
				uchar* pRow = pDataSource + widthStep * (i + 1);
				uchar* pRowDes = pDataDes + widthStepDes * i;
				for (int j = 0; j < nCols; j++) 
				{
					for (int z = 0; z < nChanel; z++) 
					{
						pRowDes[z] = pRow[z];
					}
					pRow -= nChanel;
					pRowDes += nChanelDes;
				}
			}
		}
		//Lấy đối xứng qa Oy, tương đương với CmdArguments = 0
		else if (direction == 1) 
		{
			for (int i = 0; i < nRows; i++) 
			{
				uchar* pRow = pDataSource + i * widthStep;
				uchar* pRowDes = pDataDes + (nRows - i - 1) * widthStepDes;
				for (int j = 0; j < nCols; j++) 
				{
					for (int z = 0; z < nChanel; z++) 
					{
						pRowDes[z] = pRow[z];
					}
					pRow += nChanel;
					pRowDes += nChanelDes;
				}
			}

		}
		AffineTransform u = AffineTransform();
		GeometricTransformer z = GeometricTransformer();
		interpolator->setAffineMatrix(&u);
		z.Transform(dest, dstImage, &u, interpolator);
		return 1;
	}

	GeometricTransformer() {}
	~GeometricTransformer() {}
};
