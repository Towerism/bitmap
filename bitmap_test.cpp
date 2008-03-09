/*
 ***************************************************************************
 *                                                                         *
 *                         Platform Independent                            *
 *                   Bitmap Image Reader Writer Library                    *
 *                                                                         *
 * Author: Arash Partow - 2002                                             *
 * URL: http://www.partow.net                                              *
 *                                                                         *
 * Copyright notice:                                                       *
 * Free use of the Platform Independent Bitmap Image Reader Writer Library *
 * is permitted under the guidelines and in accordance with the most       *
 * current version of the Common Public License.                           *
 * http://www.opensource.org/licenses/cpl.php                              *
 *                                                                         *
 ***************************************************************************
*/

#include <iostream>
#include <string>
#include "bitmap_image.hpp"

inline void test01()
{
   std::string file_name = "image.bmp";
   bitmap_image image("image.bmp");
   image.save_image("saved.bmp");
}

inline void test02()
{
   std::string file_name = "image.bmp";
   bitmap_image image("image.bmp");
   image.save_image("saved.bmp");

   image.vertical_flip();
   image.save_image("saved_vert_flip.bmp");
   image.vertical_flip();

   image.horizontal_flip();
   image.save_image("saved_horiz_flip.bmp");
}

inline void test03()
{
   std::string file_name = "image.bmp";
   bitmap_image image("image.bmp");
   bitmap_image subsampled_image1;
   bitmap_image subsampled_image2;
   bitmap_image subsampled_image3;

   image.subsample(subsampled_image1);
   subsampled_image1.save_image("1xsubsampled_image.bmp");

   subsampled_image1.subsample(subsampled_image2);
   subsampled_image2.save_image("2xsubsampled_image.bmp");

   subsampled_image2.subsample(subsampled_image3);
   subsampled_image3.save_image("3xsubsampled_image.bmp");
}

inline void test04()
{
   std::string file_name = "image.bmp";
   bitmap_image image("image.bmp");

   bitmap_image upsampled_image1;
   bitmap_image upsampled_image2;
   bitmap_image upsampled_image3;

   image.upsample(upsampled_image1);
   upsampled_image1.save_image("1xupsampled_image.bmp");

   upsampled_image1.upsample(upsampled_image2);
   upsampled_image2.save_image("2xupsampled_image.bmp");

   upsampled_image2.upsample(upsampled_image3);
   upsampled_image3.save_image("3xupsampled_image.bmp");
}

inline void test05()
{
   std::string file_name = "image.bmp";
   bitmap_image image("image.bmp");
   image.set_all_ith_bits_low(0);
   image.save_image("lsb0_removed_saved.bmp");
   image.set_all_ith_bits_low(1);
   image.save_image("lsb01_removed_saved.bmp");
   image.set_all_ith_bits_low(2);
   image.save_image("lsb012_removed_saved.bmp");
   image.set_all_ith_bits_low(3);
   image.save_image("lsb0123_removed_saved.bmp");
   image.set_all_ith_bits_low(4);
   image.save_image("lsb01234_removed_saved.bmp");
   image.set_all_ith_bits_low(5);
   image.save_image("lsb012345_removed_saved.bmp");
   image.set_all_ith_bits_low(6);
   image.save_image("lsb0123456_removed_saved.bmp");
}

inline void test06()
{
   std::string file_name = "image.bmp";
   bitmap_image image("image.bmp");

   bitmap_image red_channel_image;
   image.export_color_plane(bitmap_image::red_plane,red_channel_image);
   red_channel_image.save_image("red_channel_image.bmp");

   bitmap_image green_channel_image;
   image.export_color_plane(bitmap_image::green_plane,green_channel_image);
   green_channel_image.save_image("green_channel_image.bmp");

   bitmap_image blue_channel_image;
   image.export_color_plane(bitmap_image::blue_plane,blue_channel_image);
   blue_channel_image.save_image("blue_channel_image.bmp");
}

inline void test07()
{
   std::string file_name = "image.bmp";
   bitmap_image image("image.bmp");
   image.convert_to_grayscale();
   image.save_image("grayscale_image.bmp");
}

inline void test08()
{
   std::string file_name = "image.bmp";
   bitmap_image image("image.bmp");

   bitmap_image image1;
   bitmap_image image2;
   bitmap_image image3;
   bitmap_image image4;

   unsigned int w = image.width();
   unsigned int h = image.height();

   if(!image.region(0,0, w / 2, h / 2,image1))
   {
      std::cout << "ERROR: upper_left_image" << std::endl;
   }

   if(!image.region((w - 1) / 2, 0, w / 2, h / 2,image2))
   {
      std::cout << "ERROR: upper_right_image" << std::endl;
   }

   if(!image.region(0,(h - 1) / 2, w / 2, h / 2,image3))
   {
      std::cout << "ERROR: lower_left_image" << std::endl;
   }

   if(!image.region((w - 1) / 2, (h - 1) / 2, w / 2, h / 2,image4))
   {
      std::cout << "ERROR: lower_right_image" << std::endl;
   }

   image1.save_image("upper_left_image.bmp");
   image2.save_image("upper_right_image.bmp");
   image3.save_image("lower_left_image.bmp");
   image4.save_image("lower_right_image.bmp");
}

inline void test09()
{
   const unsigned int dim = 1000;
   bitmap_image image(dim,dim);
   for(unsigned int x = 0; x < dim; ++x)
   {
      for(unsigned int y = 0; y < dim; ++y)
      {
         rgb_store col = color_map[(x + y) % dim];
         image.set_pixel(x,y,col.red,col.green,col.blue);
      }
   }
   image.save_image("color_map_image.bmp");
}

inline void test10()
{
   std::string file_name = "image.bmp";
   bitmap_image image("image.bmp");
   image.invert_color_planes();
   image.save_image("inverted_color_image.bmp");
}

inline void test11()
{
   std::string file_name = "image.bmp";
   bitmap_image image("image.bmp");

   for(unsigned int i = 0; i < 10; ++i)
   {
      image.add_to_color_plane(bitmap_image::red_plane,10);
      image.save_image(static_cast<char>(48 + i) + std::string("_red_inc_image.bmp"));
   }
}

inline void test12()
{
   std::string file_name = "image.bmp";
   bitmap_image image("image.bmp");

   double* y  = new double [image.pixel_count()];
   double* cb = new double [image.pixel_count()];
   double* cr = new double [image.pixel_count()];

   image.export_ycbcr(y,cb,cr);

   for(unsigned int i = 0; i < image.pixel_count(); ++i)
   {
      cb[i] = cr[i] = 0.0;
   }
   image.import_ycbcr(y,cb,cr);
   image.save_image("only_y_image.bmp");
}

inline void test13()
{
   std::string file_name = "image.bmp";
   bitmap_image image("image.bmp");

   double* y  = new double [image.pixel_count()];
   double* cb = new double [image.pixel_count()];
   double* cr = new double [image.pixel_count()];

   image.export_ycbcr(y,cb,cr);

   for(unsigned int j = 0; j < 10; ++j)
   {
      for(unsigned int i = 0; i < image.pixel_count(); ++i)
      {
         y[i] += 15.0;
      }
      image.import_ycbcr(y,cb,cr);
      image.save_image(static_cast<char>(48 + j) + std::string("_y_image.bmp"));
   }
}


int main(void)
{
   test01();
   test02();
   test03();
   test04();
   test05();
   test06();
   test07();
   test08();
   test09();
   test10();
   test11();
   test12();
   test13();
   return 0;
}
