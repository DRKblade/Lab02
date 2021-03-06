#include <iostream>
#include "GeometricTransformer.h"
int main(int argc, char* argv[])
{
	string command = argv[1];
	string interpolate = argv[2];
	string path = argv[3];
	float argument1 = atof(argv[4]);
	float argument2 = 0;
	if (argc > 5)
		argument2 = atof(argv[5]);

	Mat source = imread(path, cv::IMREAD_COLOR);
	Mat des;
	AffineTransform u = AffineTransform();
	PixelInterpolate* interpolator = NULL;
	GeometricTransformer geometricTransformer;
	if (interpolate.find("nn") != -1)
	{
		interpolator = new NearestNeighborInterpolate(&u, source);
	}
	else if (interpolate.find("bl") != -1)
	{
  	interpolator = new BilinearInterpolate(&u, source);
	}
	else return 0;
	if (command.compare("--flip") == 0)
	{
		geometricTransformer.Flip(source, des, argument1, interpolator);
	}
	else if (command.compare("--zoom") == 0)
	{
  	u.Scale(argument1, argument1);
  	geometricTransformer.Transform(source, des, &u, interpolator);
	}
	else if (command.compare("--resize") == 0)
	{
  	u.Scale(argument1 / source.cols, argument2 / source.rows);
  	geometricTransformer.Transform(source, des, &u, interpolator);
	}
	else if (command.compare("--rotate") == 0)
	{
  	u.Rotate(argument1);
  	geometricTransformer.Transform(source, des, &u, interpolator);
	}
	else if (command.compare("--rotateN") == 0)
	{
  	u.Translate(-source.cols/2, -source.rows/2);
  	u.Rotate(argument1);
  	u.Translate(source.cols/2, source.rows/2);
  	geometricTransformer.Transform(source, des, &u, interpolator, false);
	}
	if (interpolator != NULL)delete interpolator;
	namedWindow("Source Image");
	imshow("Source Image", source);
	namedWindow("Destination Image");
	imshow("Destination Image", des);
	while (waitKey(0) != 27);
	return 0;
}
