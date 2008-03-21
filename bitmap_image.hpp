/*
***************************************************************************
*                                                                         *
*                         Platform Independent                            *
*                   Bitmap Image Reader Writer Library                    *
*                                                                         *
* Author: Arash Partow - 2002                                             *
* URL: http://www.partow.net                                              *
*                                                                         *
* Note: This library only supports 24-bits per pixel bitmap format files. *
*                                                                         *
* Copyright notice:                                                       *
* Free use of the Platform Independent Bitmap Image Reader Writer Library *
* is permitted under the guidelines and in accordance with the most       *
* current version of the Common Public License.                           *
* http://www.opensource.org/licenses/cpl.php                              *
*                                                                         *
***************************************************************************
*/

#ifndef INCLUDE_BITMAP_IMAGE_HPP
#define INCLUDE_BITMAP_IMAGE_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <iterator>
#include <limits>
#include <cmath>
#include <cstdlib>


struct bitmap_file_header
{
   unsigned short type;
   unsigned int   size;
   unsigned short reserved1;
   unsigned short reserved2;
   unsigned int   off_bits;

   unsigned int struct_size()
   {
      return sizeof(type)     +
            sizeof(size)      +
            sizeof(reserved1) +
            sizeof(reserved2) +
            sizeof(off_bits);
   }
};

struct bitmap_information_header
{
   unsigned int   size;
   unsigned int   width;
   unsigned int   height;
   unsigned short planes;
   unsigned short bit_count;
   unsigned int   compression;
   unsigned int   size_image;
   unsigned int   x_pels_per_meter;
   unsigned int   y_pels_per_meter;
   unsigned int   clr_used;
   unsigned int   clr_important;

   unsigned int struct_size()
   {
      return sizeof(size)             +
             sizeof(width)            +
             sizeof(height)           +
             sizeof(planes)           +
             sizeof(bit_count)        +
             sizeof(compression)      +
             sizeof(size_image)       +
             sizeof(x_pels_per_meter) +
             sizeof(y_pels_per_meter) +
             sizeof(clr_used)         +
             sizeof(clr_important);
   }
};


inline void read_bih(std::ifstream& stream,bitmap_information_header& bih);
inline void read_bfh(std::ifstream& stream, bitmap_file_header& bfh);
inline void write_bih(std::ofstream& stream, const bitmap_information_header& bih);
inline void write_bfh(std::ofstream& stream, const bitmap_file_header& bfh);

class bitmap_image
{
public:

   enum channel_mode {
                        rgb_mode = 0,
                        bgr_mode = 1
                     };

   enum color_plane {
                       blue_plane  = 0,
                       green_plane = 1,
                       red_plane   = 2
                    };


   bitmap_image()
   : file_name_(""),
     data_(0),
     bytes_per_pixel_(3),
     length_(0),
     width_(0),
     height_(0),
     row_increment_(0),
     channel_mode_(bgr_mode)
   {
   }

   bitmap_image(const std::string& _filename)
   : file_name_(_filename),
     data_(0),
     bytes_per_pixel_(0),
     length_(0),
     width_(0),
     height_(0),
     row_increment_(0),
     channel_mode_(bgr_mode)
   {
      load_bitmap();
   }

   bitmap_image(const unsigned int width, const unsigned int height)
   : file_name_(""),
     data_(0),
     bytes_per_pixel_(3),
     length_(0),
     width_(width),
     height_(height),
     row_increment_(0),
     channel_mode_(bgr_mode)
   {
     create_bitmap();
   }

   bitmap_image(const bitmap_image& image)
   : file_name_(image.file_name_),
     data_(0),
     bytes_per_pixel_(3),
     width_(image.width_),
     height_(image.height_),
     row_increment_(0),
     channel_mode_(bgr_mode)
   {
      create_bitmap();
      std::copy(image.data_, image.data_ + image.length_, data_);
   }

   ~bitmap_image()
   {
     delete [] data_;
   }

   bitmap_image& operator = (const bitmap_image& image)
   {
      if (this != &image)
      {
         file_name_ = image.file_name_;
         bytes_per_pixel_ = image.bytes_per_pixel_;
         width_ = image.width_;
         height_ = image.height_;
         row_increment_ = 0;
         channel_mode_ = image.channel_mode_;
         create_bitmap();
         std::copy(image.data_, image.data_ + image.length_, data_);
      }
      return *this;
   }

   inline void clear(const unsigned char v = 0x00)
   {
      std::fill(data_,data_ + length_,v);
   }

   inline unsigned char red_channel(const unsigned int x, const unsigned int y) const
   {
      return data_[(y * row_increment_) + (x * bytes_per_pixel_ + 2)];
   }

   inline unsigned char green_channel(const unsigned int x, const unsigned int y) const
   {
      return data_[(y * row_increment_) + (x * bytes_per_pixel_ + 1)];
   }

   inline unsigned char blue_channel (const unsigned int x, const unsigned int y) const
   {
      return data_[(y * row_increment_) + (x * bytes_per_pixel_ + 0)];
   }

   inline void red_channel(const unsigned int x, const unsigned int y, const unsigned char value)
   {
      data_[(y * row_increment_) + (x * bytes_per_pixel_ + 2)] = value;
   }

   inline void green_channel(const unsigned int x, const unsigned int y, const unsigned char value)
   {
      data_[(y * row_increment_) + (x * bytes_per_pixel_ + 1)] = value;
   }

   inline void blue_channel (const unsigned int x, const unsigned int y, const unsigned char value)
   {
      data_[(y * row_increment_) + (x * bytes_per_pixel_ + 0)] = value;
   }

   inline unsigned char* row(unsigned int row_index) const
   {
      return data_ + (row_index * row_increment_);
   }

   inline void get_pixel(const unsigned int x, const unsigned int y,
                         unsigned char& red,
                         unsigned char& green,
                         unsigned char& blue)
   {
      blue  = data_[(y * row_increment_) + (x * bytes_per_pixel_ + 0)];
      green = data_[(y * row_increment_) + (x * bytes_per_pixel_ + 1)];
      red   = data_[(y * row_increment_) + (x * bytes_per_pixel_ + 2)];
   }

   inline void set_pixel(const unsigned int x, const unsigned int y,
                        const unsigned char red,
                        const unsigned char green,
                        const unsigned char blue)
   {
      data_[(y * row_increment_) + (x * bytes_per_pixel_ + 0)] = blue;
      data_[(y * row_increment_) + (x * bytes_per_pixel_ + 1)] = green;
      data_[(y * row_increment_) + (x * bytes_per_pixel_ + 2)] = red;
   }

   inline bool copy_from(const bitmap_image& image)
   {
      if ((image.height_ !=  height_) ||
          (image.width_  !=  width_))
      {
         return false;
      }
      std::copy(image.data_,image.data_ + image.length_,data_);
      return true;
   }

   inline bool copy_from(const bitmap_image& source_image,
                         const unsigned int& x_offset,
                         const unsigned int& y_offset)
   {
      if ((x_offset + source_image.width_) > width_)   { return false; }
      if ((y_offset + source_image.height_) > height_) { return false; }

      for(unsigned int y = 0; y < source_image.height_; ++y)
      {
         unsigned char* it1           = row(y + y_offset) + x_offset * bytes_per_pixel_;
         const unsigned char* it2     = source_image.row(y);
         const unsigned char* it2_end = it2 + source_image.width_ * bytes_per_pixel_;
         std::copy(it2,it2_end,it1);
      }
      return true;
   }

   inline bool region(const unsigned int x,
                      const unsigned int y,
                      const unsigned int width,
                      const unsigned int height,
                      bitmap_image& dest_image)
   {
      if ((x + width) >= width_)   { return false; }
      if ((y + height) >= height_) { return false; }

      if ((dest_image.width_  < width_ ) ||
          (dest_image.height_ < height_))
      {
         dest_image.setwidth_height(width,height);
      }

      for(unsigned int r = 0; r < height; ++r)
      {
         unsigned char* it1     = row(r + y) + x * bytes_per_pixel_;
         unsigned char* it1_end = it1 + (width * bytes_per_pixel_);
         unsigned char* it2     = dest_image.row(r);
         std::copy(it1,it1_end,it2);
      }
      return true;
   }

   inline unsigned int width()  const { return width_;  }
   inline unsigned int height() const { return height_; }
   inline unsigned int bytes_per_pixel() const { return bytes_per_pixel_; }
   inline unsigned int pixel_count() const { return width_ *  height_; }

   inline void setwidth_height(const unsigned int width,
                               const unsigned int height,
                               const bool clear = false)
   {
      delete[] data_;
      width_  = width;
      height_ = height;
      create_bitmap();
      if (clear)
      {
         std::fill(data_,data_ + length_,0x00);
      }
   }

   void save_image(const std::string& file_name)
   {
      std::ofstream stream(file_name.c_str(),std::ios::binary);
      if(!stream)
      {
         std::cout << "bitmap_image::save_image(): Error - Could not open file "  << file_name << " for writing!" << std::endl;
         return;
      }

      bitmap_file_header bfh;
      bitmap_information_header bih;

      bih.width            = width_;
      bih.height           = height_;
      bih.bit_count        = static_cast<unsigned short>(bytes_per_pixel_ << 3);
      bih.clr_important    =  0;
      bih.clr_used         =  0;
      bih.compression      =  0;
      bih.planes           =  1;
      bih.size             = 40;
      bih.x_pels_per_meter =  0;
      bih.y_pels_per_meter =  0;
      bih.size_image       = (((bih.width * bytes_per_pixel_) + 3) & 0xFFFC) * bih.height;

      bfh.type      = 19778;
      bfh.size      = 55 + bih.size_image;
      bfh.reserved1 = 0;
      bfh.reserved2 = 0;
      bfh.off_bits  = bih.struct_size() + bfh.struct_size();

      write_bfh(stream,bfh);
      write_bih(stream,bih);

      unsigned int padding = (4 - ((3 * width_) % 4)) % 4;
      char padding_data[4] = {0x0,0x0,0x0,0x0};
      for (unsigned int i = 0; i < height_; ++i)
      {
         unsigned char* data_ptr = data_ + (row_increment_ /*bytes_per_pixel_ * width_*/ * (height_ - i - 1));
         stream.write(reinterpret_cast<char*>(data_ptr),sizeof(unsigned char) * bytes_per_pixel_ * width_);
         stream.write(padding_data,padding);
      }
      stream.close();
   }

   inline void set_all_ith_bits_low(const unsigned int bit_index)
   {
      unsigned char mask = static_cast<unsigned char>(~(1 << bit_index));
      for(unsigned char* it = data_; it != data_ + length_; ++it) { *it &= mask; }
   }

   inline void set_all_ith_bits_high(const unsigned int bit_index)
   {
      unsigned char mask = static_cast<unsigned char>(1 << bit_index);
      for(unsigned char* it = data_; it != data_ + length_; ++it) { *it |= mask; }
   }

   inline void set_all_ith_channels(const unsigned int& channel, const unsigned char& value)
   {
      for(unsigned char* it = (data_ + channel); it < (data_ + length_); it += bytes_per_pixel_)
      {
         *it = value;
      }
   }

   inline void set_channel(const color_plane color,const unsigned char& value)
   {
      unsigned int offset = 0;
      switch (color)
      {
         case red_plane   : offset = 2; break;
         case green_plane : offset = 1; break;
         case blue_plane  : offset = 0; break;
         default          : return;
      }
      for(unsigned char* it = (data_ + offset); it < (data_ + length_); it += bytes_per_pixel_)
      {
         *it = value;
      }
   }

   inline void ror_channel(const color_plane color, const unsigned int& ror)
   {
      unsigned int offset = 0;
      switch (color)
      {
         case red_plane   : offset = 2; break;
         case green_plane : offset = 1; break;
         case blue_plane  : offset = 0; break;
         default          : return;
      }
      for(unsigned char* it = (data_ + offset); it < (data_ + length_); it += bytes_per_pixel_)
      {
         *it = static_cast<unsigned char>(((*it) >> ror) | ((*it) << (8 - ror)));
      }
   }

   inline void set_all_channels(const unsigned char& value)
   {
      for(unsigned char* it = data_; it < (data_ + length_); )
      {
         *(it++) = value;
      }
   }

   inline void set_all_channels(const unsigned char& r_value,
                                const unsigned char& g_value,
                                const unsigned char& b_value)
   {
      for(unsigned char* it = (data_ + 0); it < (data_ + length_); it += bytes_per_pixel_)
      {
         *(it + 0) = b_value;
         *(it + 1) = g_value;
         *(it + 2) = r_value;
      }
   }

   inline void invert_color_planes()
   {
      for(unsigned char* it = data_; it < (data_ + length_); *it = ~(*it), ++it);
   }

   inline void add_to_color_plane(const color_plane color,const unsigned char& value)
   {
      for(unsigned char* it = (data_ + offset(color)); it < (data_ + length_); (*it) += value, it += bytes_per_pixel_);
   }

   inline void convert_to_grayscale()
   {
      double r_scaler = 0.299;
      double g_scaler = 0.587;
      double b_scaler = 0.114;

      if (rgb_mode == channel_mode_)
      {
         double tmp = r_scaler;
         r_scaler =  b_scaler;
         b_scaler = tmp;
      }

      for(unsigned char* it = data_; it < (data_ + length_);)
      {
         unsigned char gray_value = static_cast<unsigned char>(
         (r_scaler * (*(it + 2))) +
         (g_scaler * (*(it + 1))) +
         (b_scaler * (*(it + 0))) );
         *(it++) = gray_value;
         *(it++) = gray_value;
         *(it++) = gray_value;
      }
   }

   const unsigned char* data() { return data_; }

   inline void bgr_to_rgb()
   {
      if ((bgr_mode == channel_mode_) && (3 == bytes_per_pixel_))
      {
         reverse_channels();
         channel_mode_ = rgb_mode;
      }
   }

   inline void rgb_to_bgr()
   {
      if ((rgb_mode == channel_mode_) && (3 == bytes_per_pixel_))
      {
         reverse_channels();
         channel_mode_ = bgr_mode;
      }
   }

   void reverse()
   {
      unsigned char* it1 = data_;
      unsigned char* it2 = (data_ + length_) - bytes_per_pixel_;

      while(it1 < it2)
      {
         for(std::size_t i = 0; i < bytes_per_pixel_; ++i)
         {
            unsigned char* cit1 = it1 + i;
            unsigned char* cit2 = it2 + i;
            unsigned char tmp = *cit1;
            *cit1 = *cit2;
            *cit2 = tmp;
         }
         it1 += bytes_per_pixel_;
         it2 -= bytes_per_pixel_;
      }
   }

   void horizontal_flip()
   {
      for(unsigned int y = 0; y < height_; ++y)
      {
         unsigned char* it1 =  row(y);
         unsigned char* it2 = it1 + row_increment_ - bytes_per_pixel_;
         while(it1 < it2)
         {
            for(unsigned int i = 0; i < bytes_per_pixel_; ++i)
            {
               unsigned char* p1 = (it1 + i);
               unsigned char* p2 = (it2 + i);
               unsigned char tmp = *p1;
               *p1 = *p2;
               *p2 = tmp;
            }
            it1 += bytes_per_pixel_;
            it2 -= bytes_per_pixel_;
         }
      }
   }

   void vertical_flip()
   {
      for(unsigned int y = 0; y < (height_ / 2); ++y)
      {
         unsigned char* it1 = row(y);
         unsigned char* it2 = row(height_ - y - 1);

         for(std::size_t x = 0; x < row_increment_; ++x)
         {
            unsigned char tmp = *(it1 + x);
            *(it1 + x) = *(it2 + x);
            *(it2 + x) = tmp;
         }
      }
   }

   void export_color_plane(const color_plane color, unsigned char* image)
   {
      for(unsigned char* it = (data_ + offset(color)); it < (data_ + length_); ++image, it += bytes_per_pixel_)
      {
         (*image) = (*it);
      }
   }

   void export_color_plane(const color_plane color, bitmap_image& image)
   {
      if ((width_ != image.width_) || (height_ != image.height_))
      {
         image.setwidth_height(width_,height_);
      }

      image.clear();
      unsigned char* it1     = (data_ + offset(color));
      unsigned char* it1_end = (data_ + length_);
      unsigned char* it2     = (image.data_ + offset(color));

      while(it1 < it1_end)
      {
         (*it2) = (*it1);
         it1 += bytes_per_pixel_;
         it2 += bytes_per_pixel_;
      }
   }

   void export_response_image(const color_plane color, double* response_image)
   {
      for(unsigned char* it = (data_ + offset(color)); it < (data_ + length_); ++response_image, it += bytes_per_pixel_)
      {
         (*response_image) = (1.0 * (*it)) / 256.0;
      }
   }

   void export_gray_scale_response_image(double* response_image)
   {
      for(unsigned char* it = data_; it < (data_ + length_); it += bytes_per_pixel_)
      {
         unsigned char gray_value = static_cast<unsigned char>((0.299 * (*(it + 2))) +
                                                               (0.587 * (*(it + 1))) +
                                                               (0.114 * (*(it + 0))));
         (*response_image) = (1.0 * gray_value) / 256.0;
      }
   }

   void export_rgb(double* red, double* green, double* blue) const
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++red, ++green, ++blue)
      {
         (*blue)  = (1.0 * (*(it++))) / 256.0;
         (*green) = (1.0 * (*(it++))) / 256.0;
         (*red)   = (1.0 * (*(it++))) / 256.0;
      }
   }

   void export_rgb(float* red, float* green, float* blue) const
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++red, ++green, ++blue)
      {
         (*blue)  = (1.0f * (*(it++))) / 256.0f;
         (*green) = (1.0f * (*(it++))) / 256.0f;
         (*red)   = (1.0f * (*(it++))) / 256.0f;
      }
   }

   void export_rgb(unsigned char* red, unsigned char* green, unsigned char* blue) const
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++red, ++green, ++blue)
      {
         (*blue)  = *(it++);
         (*green) = *(it++);
         (*red)   = *(it++);
      }
   }

   void export_ycbcr(double* y, double* cb, double* cr)
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++y, ++cb, ++cr)
      {
         double blue  = (1.0 * (*(it++)));
         double green = (1.0 * (*(it++)));
         double red   = (1.0 * (*(it++)));
         ( *y) = clamp<double>( 16.0 + (1.0/256.0) * (  65.738 * red + 129.057 * green +  25.064 * blue),1.0,254);
         (*cb) = clamp<double>(128.0 + (1.0/256.0) * (- 37.945 * red -  74.494 * green + 112.439 * blue),1.0,254);
         (*cr) = clamp<double>(128.0 + (1.0/256.0) * ( 112.439 * red -  94.154 * green -  18.285 * blue),1.0,254);
      }
   }

   void export_rgb_normal(double* red, double* green, double* blue) const
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++red, ++green, ++blue)
      {
         (*blue)  = (1.0 * (*(it++)));
         (*green) = (1.0 * (*(it++)));
         (*red)   = (1.0 * (*(it++)));
      }
   }

   void export_rgb_normal(float* red, float* green, float* blue) const
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++red, ++green, ++blue)
      {
         (*blue)  = (1.0f * (*(it++)));
         (*green) = (1.0f * (*(it++)));
         (*red)   = (1.0f * (*(it++)));
      }
   }

   void import_rgb(double* red, double* green, double* blue)
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++red, ++green, ++blue)
      {
         *(it++) = static_cast<unsigned char>(256.0 * (*blue) );
         *(it++) = static_cast<unsigned char>(256.0 * (*green));
         *(it++) = static_cast<unsigned char>(256.0 * (*red)  );
      }
   }

   void import_rgb(float* red, float* green, float* blue)
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++red, ++green, ++blue)
      {
         *(it++) = static_cast<unsigned char>(256.0f * (*blue) );
         *(it++) = static_cast<unsigned char>(256.0f * (*green));
         *(it++) = static_cast<unsigned char>(256.0f * (*red)  );
      }
   }

   void import_rgb(unsigned char* red, unsigned char* green, unsigned char* blue)
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++red, ++green, ++blue)
      {
         *(it++) = (*blue );
         *(it++) = (*green);
         *(it++) = (*red  );
      }
   }

   void import_ycbcr(double* y, double* cb, double* cr)
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++y, ++cb, ++cr)
      {
         double y_  =  (*y);
         double cb_ = (*cb);
         double cr_ = (*cr);
         *(it++) = static_cast<unsigned char>(clamp((298.082 * y_ + 516.412 * cb_                 ) / 256.000 - 276.836,0.0,255.0));
         *(it++) = static_cast<unsigned char>(clamp((298.082 * y_ - 100.291 * cb_ - 208.120 * cr_ ) / 256.000 + 135.576,0.0,255.0));
         *(it++) = static_cast<unsigned char>(clamp((298.082 * y_                 + 408.583 * cr_ ) / 256.000 - 222.921,0.0,255.0));
      }
   }

   void import_rgb_clamped(double* red, double* green, double* blue)
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++red, ++green, ++blue)
      {
         *(it++) = static_cast<unsigned char>(clamp<double>(256.0 * (*blue) ,0.0,255.0));
         *(it++) = static_cast<unsigned char>(clamp<double>(256.0 * (*green),0.0,255.0));
         *(it++) = static_cast<unsigned char>(clamp<double>(256.0 * (*red)  ,0.0,255.0));
      }
   }

   void import_rgb_clamped(float* red, float* green, float* blue)
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++red, ++green, ++blue)
      {
         *(it++) = static_cast<unsigned char>(clamp<double>(256.0f * (*blue) ,0.0,255.0));
         *(it++) = static_cast<unsigned char>(clamp<double>(256.0f * (*green),0.0,255.0));
         *(it++) = static_cast<unsigned char>(clamp<double>(256.0f * (*red)  ,0.0,255.0));
      }
   }

   void import_rgb_normal(double* red, double* green, double* blue)
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++red, ++green, ++blue)
      {
         *(it++) = static_cast<unsigned char>(*blue );
         *(it++) = static_cast<unsigned char>(*green);
         *(it++) = static_cast<unsigned char>(*red  );
      }
   }

   void import_rgb_normal(float* red, float* green, float* blue)
   {
      if (bgr_mode != channel_mode_) return;
      for(unsigned char* it = data_; it < (data_ + length_); ++red, ++green, ++blue)
      {
         *(it++) = static_cast<unsigned char>(*blue );
         *(it++) = static_cast<unsigned char>(*green);
         *(it++) = static_cast<unsigned char>(*red  );
      }
   }

   void subsample(bitmap_image& dest)
   {
      /*
         Half sub-sample of original image.
      */
      unsigned int w = 0;
      unsigned int h = 0;

      bool odd_width = false;
      bool odd_height = false;

      if (0 == (width_ % 2))
         w = width_ / 2;
      else
      {
         w = 1 + (width_ / 2);
         odd_width = true;
      }

      if (0 == (height_ % 2))
         h = height_ / 2;
      else
      {
         h = 1 + (height_ / 2);
         odd_height = true;
      }
      unsigned int horizontal_upper = (odd_width)  ? w - 1 : w;
      unsigned int vertical_upper   = (odd_height) ? h - 1 : h;

      dest.setwidth_height(w,h);
      dest.clear();

            unsigned char* s_it[3];
      const unsigned char*  it1[3];
      const unsigned char*  it2[3];

      s_it[0] = dest.data_ + 0;
      s_it[1] = dest.data_ + 1;
      s_it[2] = dest.data_ + 2;

      it1[0] = data_ + 0;
      it1[1] = data_ + 1;
      it1[2] = data_ + 2;

      it2[0] = data_ + row_increment_ + 0;
      it2[1] = data_ + row_increment_ + 1;
      it2[2] = data_ + row_increment_ + 2;

      unsigned int total = 0;
      for(unsigned int j = 0; j < vertical_upper; ++j)
      {
         for (unsigned int i = 0; i < horizontal_upper; ++i)
         {
            for(unsigned int k = 0; k < bytes_per_pixel_; s_it[k] += bytes_per_pixel_, ++k)
            {
               total = 0;
               total += *(it1[k]); it1[k] += bytes_per_pixel_;
               total += *(it1[k]); it1[k] += bytes_per_pixel_;
               total += *(it2[k]); it2[k] += bytes_per_pixel_;
               total += *(it2[k]); it2[k] += bytes_per_pixel_;
               *(s_it[k]) = static_cast<unsigned char>(total >> 2);
            }
         }
         if (odd_width)
         {
            for(unsigned int k = 0; k < bytes_per_pixel_; s_it[k] += bytes_per_pixel_, ++k)
            {
               total = 0;
               total += *(it1[k]); it1[k] += bytes_per_pixel_;
               total += *(it2[k]); it2[k] += bytes_per_pixel_;
               *(s_it[k]) = static_cast<unsigned char>(total >> 1);
            }
         }

         for(unsigned int k = 0; k < bytes_per_pixel_; it1[k] += row_increment_, ++k);

         if (j != (vertical_upper - 1))
         {
            for(unsigned int k = 0; k < bytes_per_pixel_; it2[k] += row_increment_, ++k);
         }
      }
      if (odd_height)
      {
         for (unsigned int i = 0; i < horizontal_upper; ++i)
         {
            for(unsigned int k = 0; k < bytes_per_pixel_; s_it[k] += bytes_per_pixel_, ++k)
            {
               total = 0;
               total += *(it1[k]); it1[k] += bytes_per_pixel_;
               total += *(it2[k]); it2[k] += bytes_per_pixel_;
               *(s_it[k]) = static_cast<unsigned char>(total >> 1);
            }
         }
         if (odd_width)
         {
            for(unsigned int k = 0; k < bytes_per_pixel_; ++k)
            {
               (*(s_it[k])) = *(it1[k]);
            }
         }
      }
   }

   void upsample(bitmap_image& dest)
   {
      /*
         2x up-sample of original image.
      */

      dest.setwidth_height(2 * width_ ,2 * height_);
      dest.clear();

      const unsigned char* s_it[3];
            unsigned char*  it1[3];
            unsigned char*  it2[3];

      s_it[0] = data_ + 0;
      s_it[1] = data_ + 1;
      s_it[2] = data_ + 2;

      it1[0] = dest.data_ + 0;
      it1[1] = dest.data_ + 1;
      it1[2] = dest.data_ + 2;

      it2[0] = dest.data_ + dest.row_increment_ + 0;
      it2[1] = dest.data_ + dest.row_increment_ + 1;
      it2[2] = dest.data_ + dest.row_increment_ + 2;

      for(unsigned int j = 0; j < height_; ++j)
      {
         for (unsigned int i = 0; i < width_; ++i)
         {
            for(unsigned int k = 0; k < bytes_per_pixel_; s_it[k] += bytes_per_pixel_, ++k)
            {
               *(it1[k]) = *(s_it[k]); it1[k] += bytes_per_pixel_;
               *(it1[k]) = *(s_it[k]); it1[k] += bytes_per_pixel_;

               *(it2[k]) = *(s_it[k]); it2[k] += bytes_per_pixel_;
               *(it2[k]) = *(s_it[k]); it2[k] += bytes_per_pixel_;
            }
         }
         for(unsigned int k = 0; k < bytes_per_pixel_; ++k)
         {
            it1[k] += dest.row_increment_;
            it2[k] += dest.row_increment_;
         }
      }
   }

   void alpha_blend(const double& alpha, const bitmap_image& image)
   {
      if ((image.width_ != width_) || (image.height_ != height_))
      {
         return;
      }

      if ((alpha < 0.0) || (alpha > 1.0))
      {
         return;
      }

      unsigned char* it1     = data_;
      unsigned char* it1_end = data_ + length_;
      unsigned char* it2     = image.data_;

      double alpha_compliment = 1.0 - alpha;

      while(it1 != it1_end)
      {
         *(it1) = static_cast<unsigned char>((alpha * (*it2)) + (alpha_compliment * (*it1)));
         *(it1++);
         *(it2++);
      }
   }

   double psnr(const bitmap_image& image)
   {
      if ((image.width_ != width_) || (image.height_ != height_))
      {
         return 0.0;
      }

      unsigned char* it1 = data_;
      unsigned char* it2 = image.data_;

      double mse = 0.0;

      while (it1 < (data_ + length_))
      {
         double v = (static_cast<double>(*it1) - static_cast<double>(*it2));
         mse += v * v;
         ++it1;
         ++it2;
      }
      if (mse <= 0.0000001)
      {
         return 1000000.0;
      }
      else
      {
         mse /= (3.0 * width_ * height_);
         return 20.0 * std::log10(255.0 / std::sqrt(mse));
      }
   }

   void histogram(const color_plane color, double hist[256])
   {
      std::fill(hist,hist + 256,0.0);
      for(unsigned char* it = (data_ + offset(color)); it < (data_ + length_); it += bytes_per_pixel_)
      {
         ++hist[(*it)];
      }
   }

   void histogram_normalized(const color_plane color, double hist[256])
   {
      histogram(color,hist);

      std::fill(hist,hist + 256,0.0);
      for(unsigned char* it = (data_ + offset(color)); it < (data_ + length_); it += bytes_per_pixel_)
      {
         ++hist[(*it)];
      }
   }

   unsigned int offset(const color_plane color)
   {
      switch(channel_mode_)
      {
         case rgb_mode : {
                            switch (color)
                            {
                               case red_plane   : return 0;
                               case green_plane : return 1;
                               case blue_plane  : return 2;
                               default          : return std::numeric_limits<unsigned int>::max();
                            }
                         }

         case bgr_mode : {
                            switch (color)
                            {
                               case red_plane   : return 2;
                               case green_plane : return 1;
                               case blue_plane  : return 0;
                               default          : return std::numeric_limits<unsigned int>::max();
                            }
                         }
         default       : return std::numeric_limits<unsigned int>::max();
      }
   }

   void incremental()
   {
      unsigned char current_color = 0;
      for(unsigned char* it = data_; it < (data_ + length_);)
      {
         (*it++) = (current_color);
         (*it++) = (current_color);
         (*it++) = (current_color);
         ++current_color;
      }
   }

private:

   void create_bitmap()
   {
      length_ = width_ * height_ * bytes_per_pixel_;
      row_increment_ = width_ * bytes_per_pixel_;
      if (0 != data_)
      {
         delete[] data_;
      }
      data_ = new unsigned char[length_];
      valid_ = true;
   }

   void load_bitmap()
   {
      std::ifstream stream(file_name_.c_str(),std::ios::binary);
      if (!stream)
      {
         std::cerr << "bitmap_image::load_bitmap() ERROR: bitmap_image - file " << file_name_ << " not found!" << std::endl;
         return;
      }

      bitmap_file_header bfh;
      bitmap_information_header bih;

      read_bfh(stream,bfh);
      read_bih(stream,bih);

      if(bfh.type != 19778)
      {
         stream.close();
         std::cerr << "bitmap_image::load_bitmap() ERROR: bitmap_image - Invalid type value " << bfh.type << " expected 19778." << std::endl;
         return;
      }

      if(bih.bit_count != 24)
      {
         stream.close();
         std::cerr << "bitmap_image::load_bitmap() ERROR: bitmap_image - Invalid bit depth " << bih.bit_count << " expected 24." << std::endl;
         return;
      }

      height_ = bih.height;
      width_  = bih.width;

      bytes_per_pixel_ = bih.bit_count >> 3;

      unsigned int padding = (4 - ((3 * width_) % 4)) % 4;
      char padding_data[4] = {0,0,0,0};

      create_bitmap();

      for (unsigned int i = 0; i < height_; i++)
      {
         unsigned char* data_ptr = row(height_ - i - 1); // read in inverted row order
         stream.read(reinterpret_cast<char*>(data_ptr),sizeof(char) * bytes_per_pixel_ * width_);
         stream.read(padding_data,padding);
      }

      valid_ = true;
   }

   void reverse_channels()
   {
      if(3 != bytes_per_pixel_) return;
      for(unsigned char* it = data_; it < (data_ + length_); it += bytes_per_pixel_)
      {
         unsigned char tmp = *(it + 0);
         *(it + 0) = *(it + 2);
         *(it + 2) = tmp;
      }
   }

   template<typename T>
   T clamp(const T& v, const T& lower_range, const T& upper_range)
   {
      if (v < lower_range)
          return lower_range;
      else if (v >  upper_range)
         return upper_range;
      else
         return v;
   }

   bool           valid_;
   std::string    file_name_;
   unsigned char* data_;
   unsigned int   bytes_per_pixel_;
   unsigned int   length_;
   unsigned int   width_;
   unsigned int   height_;
   unsigned int   row_increment_;
   channel_mode   channel_mode_;
};


/* Utility Routines */
inline bool big_endian()
{
   unsigned int v = 0x01;
   return (1 != reinterpret_cast<char*>(&v)[0]);
}

inline unsigned short flip(const unsigned short& v)
{
   return ((v >> 8) | (v << 8));
}

inline unsigned int flip(const unsigned int& v)
{
   return (((v & 0xFF000000) >> 0x18) | ((v & 0x000000FF) << 0x18) |
          ((v & 0x00FF0000) >> 0x08) | ((v & 0x0000FF00) << 0x08));
}

template<typename T>
inline void read_from_stream(std::ifstream& stream,T& t)
{
   stream.read(reinterpret_cast<char*>(&t),sizeof(T));
}

template<typename T> inline void write_to_stream(std::ofstream& stream,const T& t)
{
   stream.write(reinterpret_cast<const char*>(&t),sizeof(T));
}


inline void read_bfh(std::ifstream& stream, bitmap_file_header& bfh)
{
   read_from_stream(stream,bfh.type);
   read_from_stream(stream,bfh.size);
   read_from_stream(stream,bfh.reserved1);
   read_from_stream(stream,bfh.reserved2);
   read_from_stream(stream,bfh.off_bits);
   if(big_endian())
   {
      flip(bfh.type);
      flip(bfh.size);
      flip(bfh.reserved1);
      flip(bfh.reserved2);
      flip(bfh.off_bits);
   }
}

inline void write_bfh(std::ofstream& stream, const bitmap_file_header& bfh)
{
   if(big_endian())
   {
      flip(bfh.type);
      flip(bfh.size);
      flip(bfh.reserved1);
      flip(bfh.reserved2);
      flip(bfh.off_bits);
   }
   write_to_stream(stream,bfh.type);
   write_to_stream(stream,bfh.size);
   write_to_stream(stream,bfh.reserved1);
   write_to_stream(stream,bfh.reserved2);
   write_to_stream(stream,bfh.off_bits);
}


inline void read_bih(std::ifstream& stream,bitmap_information_header& bih)
{
   read_from_stream(stream,bih.size);
   read_from_stream(stream,bih.width);
   read_from_stream(stream,bih.height);
   read_from_stream(stream,bih.planes);
   read_from_stream(stream,bih.bit_count);
   read_from_stream(stream,bih.compression);
   read_from_stream(stream,bih.size_image);
   read_from_stream(stream,bih.x_pels_per_meter);
   read_from_stream(stream,bih.y_pels_per_meter);
   read_from_stream(stream,bih.clr_used);
   read_from_stream(stream,bih.clr_important);
   if(big_endian())
   {
      flip(bih.size);
      flip(bih.width);
      flip(bih.height);
      flip(bih.planes);
      flip(bih.bit_count);
      flip(bih.compression);
      flip(bih.size_image);
      flip(bih.x_pels_per_meter);
      flip(bih.y_pels_per_meter);
      flip(bih.clr_used);
      flip(bih.clr_important);
   }
}

inline void write_bih(std::ofstream& stream, const bitmap_information_header& bih)
{
   if(big_endian())
   {
      flip(bih.size);
      flip(bih.width);
      flip(bih.height);
      flip(bih.planes);
      flip(bih.bit_count);
      flip(bih.compression);
      flip(bih.size_image);
      flip(bih.x_pels_per_meter);
      flip(bih.y_pels_per_meter);
      flip(bih.clr_used);
      flip(bih.clr_important);
   }
   write_to_stream(stream,bih.size);
   write_to_stream(stream,bih.width);
   write_to_stream(stream,bih.height);
   write_to_stream(stream,bih.planes);
   write_to_stream(stream,bih.bit_count);
   write_to_stream(stream,bih.compression);
   write_to_stream(stream,bih.size_image);
   write_to_stream(stream,bih.x_pels_per_meter);
   write_to_stream(stream,bih.y_pels_per_meter);
   write_to_stream(stream,bih.clr_used);
   write_to_stream(stream,bih.clr_important);
}

struct rgb_store
{
   unsigned char red;
   unsigned char green;
   unsigned char blue;
};

/* MatLab Jet color map
   Range [0-1000) or [0.0,1000.0)
*/
const rgb_store color_map[1000] = {
   { 29,   0, 102}, { 23,   0, 107}, { 17,   0, 112}, { 12,   0, 117}, {  6,   0, 122},
   {  0,   0, 127}, {  0,   0, 128}, {  0,   0, 129}, {  0,   0, 129}, {  0,   0, 130},
   {  0,   0, 131}, {  0,   0, 132}, {  0,   0, 133}, {  0,   0, 133}, {  0,   0, 134},
   {  0,   0, 135}, {  0,   0, 136}, {  0,   0, 137}, {  0,   0, 138}, {  0,   0, 140},
   {  0,   0, 141}, {  0,   0, 142}, {  0,   0, 143}, {  0,   0, 145}, {  0,   0, 146},
   {  0,   0, 147}, {  0,   0, 148}, {  0,   0, 150}, {  0,   0, 151}, {  0,   0, 152},
   {  0,   0, 153}, {  0,   0, 154}, {  0,   0, 156}, {  0,   0, 157}, {  0,   0, 158},
   {  0,   0, 159}, {  0,   0, 160}, {  0,   0, 161}, {  0,   0, 163}, {  0,   0, 164},
   {  0,   0, 165}, {  0,   0, 166}, {  0,   0, 168}, {  0,   0, 169}, {  0,   0, 170},
   {  0,   0, 171}, {  0,   0, 173}, {  0,   0, 174}, {  0,   0, 175}, {  0,   0, 176},
   {  0,   0, 178}, {  0,   0, 179}, {  0,   0, 180}, {  0,   0, 181}, {  0,   0, 183},
   {  0,   0, 184}, {  0,   0, 185}, {  0,   0, 186}, {  0,   0, 188}, {  0,   0, 189},
   {  0,   0, 190}, {  0,   0, 191}, {  0,   0, 193}, {  0,   0, 194}, {  0,   0, 195},
   {  0,   0, 196}, {  0,   0, 197}, {  0,   0, 198}, {  0,   0, 200}, {  0,   0, 201},
   {  0,   0, 202}, {  0,   0, 203}, {  0,   0, 204}, {  0,   0, 206}, {  0,   0, 207},
   {  0,   0, 208}, {  0,   0, 209}, {  0,   0, 211}, {  0,   0, 212}, {  0,   0, 213},
   {  0,   0, 214}, {  0,   0, 216}, {  0,   0, 217}, {  0,   0, 218}, {  0,   0, 219},
   {  0,   0, 221}, {  0,   0, 222}, {  0,   0, 223}, {  0,   0, 225}, {  0,   0, 226},
   {  0,   0, 227}, {  0,   0, 228}, {  0,   0, 230}, {  0,   0, 231}, {  0,   0, 232},
   {  0,   0, 233}, {  0,   0, 234}, {  0,   0, 234}, {  0,   0, 235}, {  0,   0, 236},
   {  0,   0, 237}, {  0,   0, 238}, {  0,   0, 239}, {  0,   0, 239}, {  0,   0, 240},
   {  0,   0, 241}, {  0,   0, 242}, {  0,   0, 243}, {  0,   0, 244}, {  0,   0, 246},
   {  0,   0, 247}, {  0,   0, 248}, {  0,   0, 249}, {  0,   0, 250}, {  0,   0, 251},
   {  0,   0, 253}, {  0,   0, 254}, {  0,   0, 254}, {  0,   0, 254}, {  0,   0, 254},
   {  0,   0, 254}, {  0,   0, 254}, {  0,   0, 255}, {  0,   0, 255}, {  0,   0, 255},
   {  0,   0, 255}, {  0,   0, 255}, {  0,   0, 255}, {  0,   1, 255}, {  0,   1, 255},
   {  0,   2, 255}, {  0,   3, 255}, {  0,   3, 255}, {  0,   4, 255}, {  0,   5, 255},
   {  0,   6, 255}, {  0,   6, 255}, {  0,   7, 255}, {  0,   8, 255}, {  0,   9, 255},
   {  0,  10, 255}, {  0,  11, 255}, {  0,  12, 255}, {  0,  13, 255}, {  0,  14, 255},
   {  0,  15, 255}, {  0,  16, 255}, {  0,  17, 255}, {  0,  18, 255}, {  0,  19, 255},
   {  0,  21, 255}, {  0,  22, 255}, {  0,  23, 255}, {  0,  24, 255}, {  0,  25, 255},
   {  0,  26, 255}, {  0,  27, 255}, {  0,  28, 255}, {  0,  29, 255}, {  0,  30, 255},
   {  0,  31, 255}, {  0,  32, 255}, {  0,  34, 255}, {  0,  35, 255}, {  0,  36, 255},
   {  0,  37, 255}, {  0,  38, 255}, {  0,  39, 255}, {  0,  40, 255}, {  0,  41, 255},
   {  0,  42, 255}, {  0,  43, 255}, {  0,  44, 255}, {  0,  45, 255}, {  0,  46, 255},
   {  0,  48, 255}, {  0,  49, 255}, {  0,  50, 255}, {  0,  51, 255}, {  0,  52, 255},
   {  0,  53, 255}, {  0,  54, 255}, {  0,  55, 255}, {  0,  56, 255}, {  0,  57, 255},
   {  0,  58, 255}, {  0,  58, 255}, {  0,  59, 255}, {  0,  60, 255}, {  0,  60, 255},
   {  0,  61, 255}, {  0,  62, 255}, {  0,  63, 255}, {  0,  63, 255}, {  0,  64, 255},
   {  0,  65, 255}, {  0,  66, 255}, {  0,  67, 255}, {  0,  68, 255}, {  0,  69, 255},
   {  0,  71, 255}, {  0,  72, 255}, {  0,  73, 255}, {  0,  74, 255}, {  0,  75, 255},
   {  0,  76, 255}, {  0,  77, 255}, {  0,  78, 255}, {  0,  79, 255}, {  0,  80, 255},
   {  0,  81, 255}, {  0,  82, 255}, {  0,  84, 255}, {  0,  85, 255}, {  0,  86, 255},
   {  0,  87, 255}, {  0,  88, 255}, {  0,  89, 255}, {  0,  90, 255}, {  0,  91, 255},
   {  0,  92, 255}, {  0,  93, 255}, {  0,  94, 255}, {  0,  95, 255}, {  0,  96, 255},
   {  0,  98, 255}, {  0,  99, 255}, {  0, 100, 255}, {  0, 101, 255}, {  0, 102, 255},
   {  0, 103, 255}, {  0, 104, 255}, {  0, 105, 255}, {  0, 106, 255}, {  0, 107, 255},
   {  0, 108, 255}, {  0, 109, 255}, {  0, 111, 255}, {  0, 112, 255}, {  0, 113, 255},
   {  0, 114, 255}, {  0, 115, 255}, {  0, 116, 255}, {  0, 117, 255}, {  0, 118, 255},
   {  0, 119, 255}, {  0, 120, 255}, {  0, 121, 255}, {  0, 122, 255}, {  0, 123, 255},
   {  0, 125, 255}, {  0, 126, 255}, {  0, 127, 255}, {  0, 128, 255}, {  0, 129, 255},
   {  0, 130, 255}, {  0, 131, 255}, {  0, 132, 255}, {  0, 133, 255}, {  0, 134, 255},
   {  0, 135, 255}, {  0, 136, 255}, {  0, 138, 255}, {  0, 139, 255}, {  0, 140, 255},
   {  0, 141, 255}, {  0, 142, 255}, {  0, 143, 255}, {  0, 144, 255}, {  0, 145, 255},
   {  0, 146, 255}, {  0, 147, 255}, {  0, 148, 255}, {  0, 149, 255}, {  0, 150, 255},
   {  0, 150, 255}, {  0, 151, 255}, {  0, 152, 255}, {  0, 153, 255}, {  0, 153, 255},
   {  0, 154, 255}, {  0, 155, 255}, {  0, 155, 255}, {  0, 156, 255}, {  0, 157, 255},
   {  0, 158, 255}, {  0, 159, 255}, {  0, 161, 255}, {  0, 162, 255}, {  0, 163, 255},
   {  0, 164, 255}, {  0, 165, 255}, {  0, 166, 255}, {  0, 167, 255}, {  0, 168, 255},
   {  0, 169, 255}, {  0, 170, 255}, {  0, 171, 255}, {  0, 172, 255}, {  0, 173, 255},
   {  0, 175, 255}, {  0, 176, 255}, {  0, 177, 255}, {  0, 178, 255}, {  0, 179, 255},
   {  0, 180, 255}, {  0, 181, 255}, {  0, 182, 255}, {  0, 183, 255}, {  0, 184, 255},
   {  0, 185, 255}, {  0, 186, 255}, {  0, 188, 255}, {  0, 189, 255}, {  0, 190, 255},
   {  0, 191, 255}, {  0, 192, 255}, {  0, 193, 255}, {  0, 194, 255}, {  0, 195, 255},
   {  0, 196, 255}, {  0, 197, 255}, {  0, 198, 255}, {  0, 199, 255}, {  0, 200, 255},
   {  0, 202, 255}, {  0, 203, 255}, {  0, 204, 255}, {  0, 205, 255}, {  0, 206, 255},
   {  0, 207, 255}, {  0, 208, 255}, {  0, 209, 255}, {  0, 210, 255}, {  0, 211, 255},
   {  0, 212, 255}, {  0, 213, 255}, {  0, 215, 255}, {  0, 216, 255}, {  0, 217, 255},
   {  0, 218, 254}, {  0, 219, 253}, {  0, 220, 252}, {  0, 221, 252}, {  0, 222, 251},
   {  0, 223, 250}, {  0, 224, 250}, {  0, 225, 249}, {  0, 226, 248}, {  0, 227, 247},
   {  0, 229, 247}, {  1, 230, 246}, {  2, 231, 245}, {  3, 232, 244}, {  3, 233, 243},
   {  4, 234, 242}, {  5, 235, 241}, {  5, 236, 240}, {  6, 237, 239}, {  7, 238, 238},
   {  8, 239, 238}, {  8, 240, 237}, {  9, 241, 236}, { 10, 242, 236}, { 10, 242, 235},
   { 11, 243, 235}, { 11, 244, 234}, { 12, 245, 234}, { 13, 245, 233}, { 13, 246, 232},
   { 14, 247, 232}, { 15, 247, 231}, { 15, 248, 231}, { 16, 249, 230}, { 17, 249, 229},
   { 18, 250, 228}, { 18, 251, 227}, { 19, 251, 226}, { 20, 252, 225}, { 21, 253, 224},
   { 22, 253, 224}, { 23, 254, 223}, { 23, 254, 222}, { 24, 255, 221}, { 25, 255, 220},
   { 26, 255, 219}, { 27, 255, 218}, { 28, 255, 218}, { 29, 255, 217}, { 30, 255, 216},
   { 30, 255, 215}, { 31, 255, 214}, { 32, 255, 214}, { 33, 255, 213}, { 34, 255, 212},
   { 35, 255, 211}, { 36, 255, 210}, { 37, 255, 209}, { 38, 255, 208}, { 39, 255, 207},
   { 39, 255, 207}, { 40, 255, 206}, { 41, 255, 205}, { 42, 255, 204}, { 43, 255, 203},
   { 44, 255, 202}, { 45, 255, 201}, { 46, 255, 200}, { 47, 255, 199}, { 48, 255, 198},
   { 48, 255, 198}, { 49, 255, 197}, { 50, 255, 196}, { 51, 255, 195}, { 52, 255, 194},
   { 53, 255, 193}, { 54, 255, 192}, { 55, 255, 191}, { 55, 255, 191}, { 56, 255, 190},
   { 57, 255, 189}, { 58, 255, 188}, { 59, 255, 187}, { 60, 255, 186}, { 60, 255, 186},
   { 61, 255, 185}, { 62, 255, 184}, { 63, 255, 183}, { 64, 255, 182}, { 65, 255, 181},
   { 65, 255, 181}, { 66, 255, 180}, { 67, 255, 179}, { 68, 255, 178}, { 69, 255, 177},
   { 70, 255, 176}, { 71, 255, 175}, { 72, 255, 174}, { 73, 255, 173}, { 74, 255, 172},
   { 74, 255, 172}, { 75, 255, 171}, { 76, 255, 170}, { 77, 255, 169}, { 78, 255, 168},
   { 79, 255, 167}, { 80, 255, 166}, { 81, 255, 165}, { 82, 255, 164}, { 83, 255, 163},
   { 83, 255, 163}, { 84, 255, 162}, { 84, 255, 162}, { 85, 255, 161}, { 85, 255, 161},
   { 86, 255, 160}, { 87, 255, 159}, { 87, 255, 159}, { 88, 255, 158}, { 88, 255, 158},
   { 89, 255, 157}, { 89, 255, 157}, { 90, 255, 156}, { 91, 255, 155}, { 92, 255, 154},
   { 93, 255, 153}, { 94, 255, 152}, { 95, 255, 151}, { 96, 255, 150}, { 97, 255, 149},
   { 97, 255, 149}, { 98, 255, 148}, { 99, 255, 147}, {100, 255, 146}, {101, 255, 145},
   {102, 255, 144}, {102, 255, 143}, {103, 255, 142}, {104, 255, 141}, {105, 255, 140},
   {106, 255, 140}, {107, 255, 139}, {107, 255, 138}, {108, 255, 137}, {109, 255, 136},
   {110, 255, 135}, {111, 255, 134}, {112, 255, 134}, {113, 255, 133}, {114, 255, 132},
   {114, 255, 131}, {115, 255, 130}, {116, 255, 130}, {117, 255, 129}, {118, 255, 128},
   {119, 255, 127}, {120, 255, 126}, {121, 255, 125}, {122, 255, 124}, {123, 255, 123},
   {123, 255, 123}, {124, 255, 122}, {125, 255, 121}, {126, 255, 120}, {127, 255, 119},
   {128, 255, 118}, {129, 255, 117}, {130, 255, 116}, {130, 255, 115}, {131, 255, 114},
   {132, 255, 114}, {133, 255, 113}, {134, 255, 112}, {134, 255, 111}, {135, 255, 110},
   {136, 255, 109}, {137, 255, 108}, {138, 255, 107}, {139, 255, 107}, {140, 255, 106},
   {140, 255, 105}, {141, 255, 104}, {142, 255, 103}, {143, 255, 102}, {144, 255, 102},
   {145, 255, 101}, {146, 255, 100}, {147, 255,  99}, {148, 255,  98}, {149, 255,  97},
   {149, 255,  97}, {150, 255,  96}, {151, 255,  95}, {152, 255,  94}, {153, 255,  93},
   {154, 255,  92}, {155, 255,  91}, {156, 255,  90}, {157, 255,  89}, {157, 255,  89},
   {158, 255,  88}, {158, 255,  88}, {159, 255,  87}, {159, 255,  87}, {160, 255,  86},
   {161, 255,  85}, {161, 255,  85}, {162, 255,  84}, {162, 255,  84}, {163, 255,  83},
   {163, 255,  83}, {164, 255,  82}, {165, 255,  81}, {166, 255,  80}, {167, 255,  79},
   {168, 255,  78}, {169, 255,  77}, {170, 255,  76}, {171, 255,  75}, {172, 255,  74},
   {172, 255,  74}, {173, 255,  73}, {174, 255,  72}, {175, 255,  71}, {176, 255,  70},
   {177, 255,  69}, {178, 255,  68}, {179, 255,  67}, {180, 255,  66}, {181, 255,  65},
   {181, 255,  65}, {182, 255,  64}, {183, 255,  63}, {184, 255,  62}, {185, 255,  61},
   {186, 255,  60}, {186, 255,  60}, {187, 255,  59}, {188, 255,  58}, {189, 255,  57},
   {190, 255,  56}, {191, 255,  55}, {191, 255,  55}, {192, 255,  54}, {193, 255,  53},
   {194, 255,  52}, {195, 255,  51}, {196, 255,  50}, {197, 255,  49}, {198, 255,  48},
   {198, 255,  48}, {199, 255,  47}, {200, 255,  46}, {201, 255,  45}, {202, 255,  44},
   {203, 255,  43}, {204, 255,  42}, {205, 255,  41}, {206, 255,  40}, {207, 255,  39},
   {207, 255,  39}, {208, 255,  38}, {209, 255,  37}, {210, 255,  36}, {211, 255,  35},
   {212, 255,  34}, {213, 255,  33}, {214, 255,  32}, {214, 255,  31}, {215, 255,  30},
   {216, 255,  30}, {217, 255,  29}, {218, 255,  28}, {218, 255,  27}, {219, 255,  26},
   {220, 255,  25}, {221, 255,  24}, {222, 255,  23}, {223, 255,  23}, {224, 255,  22},
   {224, 255,  21}, {225, 255,  20}, {226, 255,  19}, {227, 255,  18}, {228, 255,  18},
   {229, 255,  17}, {230, 255,  16}, {231, 255,  15}, {231, 255,  15}, {232, 255,  14},
   {232, 255,  13}, {233, 255,  13}, {234, 255,  12}, {234, 255,  11}, {235, 255,  11},
   {235, 255,  10}, {236, 255,  10}, {236, 255,   9}, {237, 255,   8}, {238, 254,   8},
   {238, 253,   7}, {239, 252,   6}, {240, 251,   5}, {241, 250,   5}, {242, 249,   4},
   {243, 248,   3}, {244, 247,   3}, {245, 246,   2}, {246, 246,   1}, {247, 245,   0},
   {247, 243,   0}, {248, 242,   0}, {249, 242,   0}, {250, 241,   0}, {250, 240,   0},
   {251, 239,   0}, {252, 238,   0}, {252, 237,   0}, {253, 236,   0}, {254, 235,   0},
   {255, 234,   0}, {255, 233,   0}, {255, 232,   0}, {255, 231,   0}, {255, 230,   0},
   {255, 229,   0}, {255, 228,   0}, {255, 227,   0}, {255, 226,   0}, {255, 225,   0},
   {255, 224,   0}, {255, 223,   0}, {255, 222,   0}, {255, 221,   0}, {255, 220,   0},
   {255, 219,   0}, {255, 218,   0}, {255, 217,   0}, {255, 216,   0}, {255, 215,   0},
   {255, 214,   0}, {255, 213,   0}, {255, 212,   0}, {255, 211,   0}, {255, 210,   0},
   {255, 209,   0}, {255, 208,   0}, {255, 207,   0}, {255, 206,   0}, {255, 205,   0},
   {255, 204,   0}, {255, 203,   0}, {255, 202,   0}, {255, 201,   0}, {255, 200,   0},
   {255, 199,   0}, {255, 198,   0}, {255, 197,   0}, {255, 196,   0}, {255, 195,   0},
   {255, 194,   0}, {255, 193,   0}, {255, 192,   0}, {255, 191,   0}, {255, 190,   0},
   {255, 189,   0}, {255, 188,   0}, {255, 187,   0}, {255, 186,   0}, {255, 185,   0},
   {255, 184,   0}, {255, 183,   0}, {255, 182,   0}, {255, 180,   0}, {255, 179,   0},
   {255, 178,   0}, {255, 177,   0}, {255, 176,   0}, {255, 176,   0}, {255, 175,   0},
   {255, 175,   0}, {255, 174,   0}, {255, 173,   0}, {255, 173,   0}, {255, 172,   0},
   {255, 171,   0}, {255, 171,   0}, {255, 170,   0}, {255, 169,   0}, {255, 168,   0},
   {255, 167,   0}, {255, 166,   0}, {255, 165,   0}, {255, 164,   0}, {255, 163,   0},
   {255, 162,   0}, {255, 161,   0}, {255, 160,   0}, {255, 159,   0}, {255, 158,   0},
   {255, 157,   0}, {255, 156,   0}, {255, 155,   0}, {255, 154,   0}, {255, 153,   0},
   {255, 152,   0}, {255, 151,   0}, {255, 150,   0}, {255, 150,   0}, {255, 149,   0},
   {255, 147,   0}, {255, 146,   0}, {255, 146,   0}, {255, 145,   0}, {255, 144,   0},
   {255, 143,   0}, {255, 142,   0}, {255, 141,   0}, {255, 140,   0}, {255, 139,   0},
   {255, 138,   0}, {255, 137,   0}, {255, 136,   0}, {255, 135,   0}, {255, 134,   0},
   {255, 133,   0}, {255, 132,   0}, {255, 131,   0}, {255, 130,   0}, {255, 129,   0},
   {255, 128,   0}, {255, 127,   0}, {255, 126,   0}, {255, 125,   0}, {255, 124,   0},
   {255, 123,   0}, {255, 122,   0}, {255, 121,   0}, {255, 120,   0}, {255, 119,   0},
   {255, 118,   0}, {255, 117,   0}, {255, 116,   0}, {255, 115,   0}, {255, 114,   0},
   {255, 113,   0}, {255, 112,   0}, {255, 111,   0}, {255, 109,   0}, {255, 108,   0},
   {255, 107,   0}, {255, 106,   0}, {255, 105,   0}, {255, 104,   0}, {255, 103,   0},
   {255, 102,   0}, {255, 101,   0}, {255, 100,   0}, {255,  99,   0}, {255,  98,   0},
   {255,  97,   0}, {255,  96,   0}, {255,  95,   0}, {255,  94,   0}, {255,  93,   0},
   {255,  92,   0}, {255,  91,   0}, {255,  91,   0}, {255,  90,   0}, {255,  90,   0},
   {255,  89,   0}, {255,  88,   0}, {255,  88,   0}, {255,  87,   0}, {255,  86,   0},
   {255,  86,   0}, {255,  85,   0}, {255,  84,   0}, {255,  83,   0}, {255,  82,   0},
   {255,  81,   0}, {255,  80,   0}, {255,  79,   0}, {255,  78,   0}, {255,  77,   0},
   {255,  76,   0}, {255,  75,   0}, {255,  74,   0}, {255,  73,   0}, {255,  72,   0},
   {255,  71,   0}, {255,  70,   0}, {255,  69,   0}, {255,  68,   0}, {255,  67,   0},
   {255,  66,   0}, {255,  65,   0}, {255,  64,   0}, {255,  63,   0}, {255,  62,   0},
   {255,  61,   0}, {255,  60,   0}, {255,  59,   0}, {255,  58,   0}, {255,  57,   0},
   {255,  56,   0}, {255,  55,   0}, {255,  54,   0}, {255,  54,   0}, {255,  53,   0},
   {255,  51,   0}, {255,  50,   0}, {255,  49,   0}, {255,  48,   0}, {255,  47,   0},
   {255,  46,   0}, {255,  45,   0}, {255,  44,   0}, {255,  43,   0}, {255,  42,   0},
   {255,  41,   0}, {255,  40,   0}, {255,  39,   0}, {255,  38,   0}, {255,  37,   0},
   {255,  36,   0}, {255,  35,   0}, {255,  34,   0}, {255,  33,   0}, {255,  32,   0},
   {255,  31,   0}, {255,  30,   0}, {255,  29,   0}, {255,  28,   0}, {255,  27,   0},
   {255,  26,   0}, {255,  25,   0}, {255,  24,   0}, {254,  23,   0}, {254,  22,   0},
   {254,  21,   0}, {254,  20,   0}, {254,  19,   0}, {254,  18,   0}, {253,  17,   0},
   {251,  16,   0}, {250,  15,   0}, {249,  14,   0}, {248,  13,   0}, {247,  12,   0},
   {246,  11,   0}, {244,  10,   0}, {243,   9,   0}, {242,   8,   0}, {241,   7,   0},
   {240,   6,   0}, {239,   6,   0}, {239,   5,   0}, {238,   4,   0}, {237,   4,   0},
   {236,   3,   0}, {235,   3,   0}, {234,   2,   0}, {234,   1,   0}, {233,   1,   0},
   {232,   0,   0}, {231,   0,   0}, {230,   0,   0}, {228,   0,   0}, {227,   0,   0},
   {226,   0,   0}, {225,   0,   0}, {223,   0,   0}, {222,   0,   0}, {221,   0,   0},
   {219,   0,   0}, {218,   0,   0}, {217,   0,   0}, {216,   0,   0}, {214,   0,   0},
   {213,   0,   0}, {212,   0,   0}, {211,   0,   0}, {209,   0,   0}, {208,   0,   0},
   {207,   0,   0}, {206,   0,   0}, {204,   0,   0}, {203,   0,   0}, {202,   0,   0},
   {201,   0,   0}, {200,   0,   0}, {198,   0,   0}, {197,   0,   0}, {196,   0,   0},
   {195,   0,   0}, {194,   0,   0}, {193,   0,   0}, {191,   0,   0}, {190,   0,   0},
   {189,   0,   0}, {188,   0,   0}, {186,   0,   0}, {185,   0,   0}, {184,   0,   0},
   {183,   0,   0}, {181,   0,   0}, {180,   0,   0}, {179,   0,   0}, {178,   0,   0},
   {176,   0,   0}, {175,   0,   0}, {174,   0,   0}, {173,   0,   0}, {171,   0,   0},
   {170,   0,   0}, {169,   0,   0}, {168,   0,   0}, {166,   0,   0}, {165,   0,   0},
   {164,   0,   0}, {163,   0,   0}, {161,   0,   0}, {160,   0,   0}, {159,   0,   0},
   {158,   0,   0}, {157,   0,   0}, {156,   0,   0}, {154,   0,   0}, {153,   0,   0},
   {152,   0,   0}, {151,   0,   0}, {150,   0,   0}, {148,   0,   0}, {147,   0,   0},
   {146,   0,   0}, {145,   0,   0}, {143,   0,   0}, {142,   0,   0}, {141,   0,   0},
   {140,   0,   0}, {138,   0,   0}, {137,   0,   0}, {136,   0,   0}, {135,   0,   0},
   {134,   0,   0}, {133,   0,   0}, {133,   0,   0}, {132,   0,   0}, {131,   0,   0},
   {130,   0,   0}, {129,   0,   0}, {129,   0,   0}, {128,   0,   0}, {127,   0,   0},
   {122,   0,   9}, {117,   0,  18}, {112,   0,  27}, {107,   0,  36}, {102,   0,  45}
};


inline void rgb_to_ycbcr(const unsigned int& length, double* red, double* green, double* blue,
                                                     double* y,   double* cb,    double* cr)
{
   unsigned int i = 0;
   while (i < length)
   {
      ( *y) =   16.0 + (  65.481 * (*red) +  128.553 * (*green) +  24.966 * (*blue));
      (*cb) =  128.0 + ( -37.797 * (*red) +  -74.203 * (*green) + 112.000 * (*blue));
      (*cr) =  128.0 + ( 112.000 * (*red) +  -93.786 * (*green) -  18.214 * (*blue));
      ++i;
      ++red; ++green; ++blue;
      ++y;   ++cb;    ++cr;
   }
}

inline void ycbcr_to_rgb(const unsigned int& length, double* y,   double* cb,    double* cr,
                                                     double* red, double* green, double* blue)
{
   unsigned int i = 0;
   while (i < length)
   {
      double y_  =  (*y) -  16.0;
      double cb_ = (*cb) - 128.0;
      double cr_ = (*cr) - 128.0;

        (*red) = 0.000456621 * y_                    + 0.00625893 * cr_;
      (*green) = 0.000456621 * y_ - 0.00153632 * cb_ - 0.00318811 * cr_;
       (*blue) = 0.000456621 * y_                    + 0.00791071 * cb_;

      ++i;
      ++red; ++green; ++blue;
      ++y;   ++cb;    ++cr;
   }
}

inline void subsample(const unsigned int& width,
                      const unsigned int& height,
                      const double* source,
                      unsigned int& w,
                      unsigned int& h,
                      double** dest)
{
   /*
      Single channel.
   */

   w = 0;
   h = 0;

   bool odd_width = false;
   bool odd_height = false;

   if (0 == (width % 2))
      w = width / 2;
   else
   {
      w = 1 + (width / 2);
      odd_width = true;
   }

   if (0 == (height % 2))
      h = height / 2;
   else
   {
      h = 1 + (height / 2);
      odd_height = true;
   }
   unsigned int horizontal_upper = (odd_width)  ? w - 1 : w;
   unsigned int vertical_upper   = (odd_height) ? h - 1 : h;

   *dest = new double[w * h];

   double* s_it = *dest;
   const double* it1 = source;
   const double* it2 = source + width;

   for(unsigned int j = 0; j < vertical_upper; ++j)
   {
      for (unsigned int i = 0; i < horizontal_upper; ++i, ++s_it)
      {
          (*s_it)  = *(it1++);
          (*s_it) += *(it1++);
          (*s_it) += *(it2++);
          (*s_it) += *(it2++);
          (*s_it) /=  4.0;
      }
      if (odd_width)
      {
         (*(s_it++)) = ( (*it1++) + (*it2++) ) / 2.0;
      }

      it1 += width;
      if (j != (vertical_upper -1))
      {
         it2 += width;
      }
   }
   if (odd_height)
   {
      for (unsigned int i = 0; i < horizontal_upper; ++i, ++s_it)
      {
         (*s_it) = ( (*(it1++)) + (*(it1++))  ) / 2.0;
      }
      if (odd_width)
      {
         (*(s_it++)) = (*it1);
      }
   }
}

inline void upsample(const unsigned int& width,
                     const unsigned int& height,
                     const double* source,
                     unsigned int& w,
                     unsigned int& h,
                     double** dest)
{
   /*
      Single channel.
   */

   w = 2 * width;
   h = 2 * height;

   *dest = new double[w * h];

   const double* s_it = source;
         double* it1 = *dest;
         double* it2 = *dest + w;

   for(unsigned int j = 0; j < height; ++j)
   {
      for (unsigned int i = 0; i < width; ++i, ++s_it)
      {
          *(it1++) = (*s_it);
          *(it1++) = (*s_it);
          *(it2++) = (*s_it);
          *(it2++) = (*s_it);
      }
      it1 += w;
      it2 += w;
   }
}

void checkered_pattern(const unsigned int x_width,
                       const unsigned int y_width,
                       const unsigned char value,
                       const bitmap_image::color_plane color,
                             bitmap_image& image)
{
   if ((x_width >= image.width()) || (y_width >= image.height()))
   {
      return;
   }

   bool setter_x = false;
   bool setter_y = true;

   for(unsigned int y = 0; y < image.height(); ++y)
   {
      if (0 == (y % y_width)) setter_y = !setter_y;

      unsigned char* row = image.row(y) + image.offset(color);
      for(unsigned int x = 0; x < image.width(); ++x, row += image.bytes_per_pixel())
      {
         if (0 == (x % x_width)) setter_x = !setter_x;
         if (setter_x && !setter_y)
         {
            *row = value;
         }
      }
   }
}

void plasma(bitmap_image& image,
            const double& x,     const double& y,
            const double& width, const double& height,
            const double& c1,    const double& c2,
            const double& c3,    const double& c4,
            const double& roughness = 3.0)
{
   // Note: c1,c2,c3,c4 -> [0.0,1.0]

   double corner1 = 0.0;
   double corner2 = 0.0;
   double corner3 = 0.0;
   double corner4 = 0.0;
   double center  = 0.0;
   double half_width  = ( width / 2.0);
   double half_height = (height / 2.0);

   if ((width >= 1.0) || (height >= 1.0))
   {
      corner1 = (c1 + c2) / 2.0;
      corner2 = (c2 + c3) / 2.0;
      corner3 = (c3 + c4) / 2.0;
      corner4 = (c4 + c1) / 2.0;
      center  = (c1 + c2 + c3 + c4) / 4.0 +
                ((1.0 * ::rand() /(1.0 * RAND_MAX))  - 0.5) * // should use a better rng
                ((1.0 * half_width + half_height) / (image.width() + image.height()) * roughness);

      center = std::min<double>(std::max<double>(0.0,center),1.0);

      plasma(image, x,                            y, half_width, half_height,      c1, corner1,  center, corner4);
      plasma(image, x + half_width,               y, half_width, half_height, corner1,      c2, corner2,  center);
      plasma(image, x + half_width, y + half_height, half_width, half_height,  center, corner2,      c3, corner3);
      plasma(image, x,              y + half_height, half_width, half_height, corner4,  center, corner3,      c4);
   }
   else
   {
      rgb_store color = color_map[static_cast<unsigned int>(1000.0 * ((c1 + c2 + c3 + c4) / 4.0)) % 1000];
      image.set_pixel(static_cast<unsigned int>(x),static_cast<unsigned int>(y),color.red,color.green,color.blue);
   }
}


class image_drawer
{
public:

   image_drawer(bitmap_image& image)
   : image_(image),
     pen_width_(1),
     pen_color_red_(0),
     pen_color_green_(0),
     pen_color_blue_(0){};

   void rectangle(int x1, int y1, int x2, int y2)
   {
      line_segment(x1,y1,x2,y1);
      line_segment(x2,y1,x2,y2);
      line_segment(x2,y2,x1,y2);
      line_segment(x1,y2,x1,y1);
   }

   void triangle(int x1, int y1, int x2, int y2,int x3, int y3)
   {
      line_segment(x1,y1,x2,y2);
      line_segment(x2,y2,x3,y3);
      line_segment(x3,y3,x1,y1);
   }

   void quadix(int x1, int y1, int x2, int y2,int x3, int y3, int x4, int y4)
   {
      line_segment(x1,y1,x2,y2);
      line_segment(x2,y2,x3,y3);
      line_segment(x3,y3,x4,y4);
      line_segment(x4,y4,x1,y1);
   }

   void line_segment(int x1, int y1, int x2, int y2)
   {
      int steep = 0;
      int sx    = ((x2 - x1) > 0) ? 1 : -1;
      int sy    = ((y2 - y1) > 0) ? 1 : -1;
      int dx    = abs(x2 - x1);
      int dy    = abs(y2 - y1);

      if(dy > dx)
      {
         steep = x1;  x1 = y1;  y1 = steep;  /* swap x1 and y1 */
         steep = dx;  dx = dy;  dy = steep;  /* swap dx and dy */
         steep = sx;  sx = sy;  sy = steep;  /* swap sx and sy */
         steep = 1;
      }

      int e = 2 * dy - dx;

      for(int i = 0; i < dx; ++i)
      {
         if(steep)
            plot_pen_pixel(y1,x1);
         else
            plot_pen_pixel(x1,y1);

         while(e >= 0)
         {
            y1 += sy;
            e -= (dx << 1);
         }
         x1 += sx;
         e  += (dy << 1);
      }
      plot_pen_pixel(x2,y2);
   }

   void horiztonal_line_segment(int x1, int x2, int y)
   {
      if (x1 > x2)
      {
         std::swap(x1,x2);
      }

      for(int i = 0; i < (x2 - x1); ++i)
      {
         plot_pen_pixel(x1 +  i,y);
      }
   }

   void vertical_line_segment(int y1, int y2, int x)
   {
      if (y1 > y2)
      {
         std::swap(y1,y2);
      }

      for(int i = 0; i < (y2 - y1); ++i)
      {
         plot_pen_pixel(x, y1 +  i);
      }
   }

   void ellipse(int centerx, int centery, int a, int b)
   {
      int t1 = a * a;
      int t2 = t1 << 1;
      int t3 = t2 << 1;
      int t4 = b * b;
      int t5 = t4 << 1;
      int t6 = t5 << 1;
      int t7 = a * t5;
      int t8 = t7 << 1;
      int t9 = 0;

      int d1 = t2 - t7 + (t4 >> 1);
      int d2 = (t1 >> 1) - t8 + t5;
      int x  = a;
      int y  = 0;

      int negative_tx = centerx - x;
      int positive_tx = centerx + x;
      int negative_ty = centery - y;
      int positive_ty = centery + y;

      while (d2 < 0)
      {
         plot_pen_pixel(positive_tx,positive_ty);
         plot_pen_pixel(positive_tx,negative_ty);
         plot_pen_pixel(negative_tx,positive_ty);
         plot_pen_pixel(negative_tx,negative_ty);

         ++y;

         t9 = t9 + t3;

         if (d1 < 0)
         {
            d1 = d1 + t9 + t2;
            d2 = d2 + t9;
         }
         else
         {
            x--;
            t8 = t8 - t6;
            d1 = d1 + (t9 + t2 - t8);
            d2 = d2 + (t9 + t5 - t8);
            negative_tx = centerx - x;
            positive_tx = centerx + x;
         }

         negative_ty = centery - y;
         positive_ty = centery + y;
      }

      do
      {
         plot_pen_pixel(positive_tx,positive_ty);
         plot_pen_pixel(positive_tx,negative_ty);
         plot_pen_pixel(negative_tx,positive_ty);
         plot_pen_pixel(negative_tx,negative_ty);

         x--;
         t8 = t8 - t6;

         if (d2 < 0)
         {
            ++y;
            t9 = t9 + t3;
            d2 = d2 + (t9 + t5 - t8);
            negative_ty = centery - y;
            positive_ty = centery + y;
         }
         else
            d2 = d2 + (t5 - t8);

         negative_tx = centerx - x;
         positive_tx = centerx + x;
      }
      while (x >= 0);
   }

   void circle(int centerx, int centery, int radius)
   {
      int x = 0;
      int d = (1 - radius) << 1;

      while(radius >= 0)
      {
         plot_pen_pixel(centerx + x,centery + radius);
         plot_pen_pixel(centerx + x,centery - radius);
         plot_pen_pixel(centerx - x,centery + radius);
         plot_pen_pixel(centerx - x,centery - radius);

         if ((d + radius) > 0)
            d -= ((--radius) << 1) - 1;
         if (x > d)
            d += ((++x) << 1) + 1;
      }
   }

   void plot_pen_pixel(int x, int y)
   {
      switch (pen_width_)
      {
         case 1  : plot_pixel(x,y);
                   break;

         case 2  : {
                      plot_pixel(x    , y    );
                      plot_pixel(x + 1, y    );
                      plot_pixel(x + 1, y + 1);
                      plot_pixel(x    , y + 1);
                   }
                   break;

         case  3 : {
                      plot_pixel(x    , y - 1);
                      plot_pixel(x - 1, y - 1);
                      plot_pixel(x + 1, y - 1);

                      plot_pixel(x    , y    );
                      plot_pixel(x - 1, y    );
                      plot_pixel(x + 1, y    );

                      plot_pixel(x    , y + 1);
                      plot_pixel(x - 1, y + 1);
                      plot_pixel(x + 1, y + 1);
                   }
                   break;

         default : plot_pixel(x,y);
                   break;
      }
   }

   void plot_pixel(int x, int y)
   {
      image_.set_pixel(x,y,pen_color_red_,pen_color_green_,pen_color_blue_);
   }

   void pen_width(const unsigned int& width)
   {
      if ((width > 0) && (width < 4))
      {
         pen_width_ = width;
      }
   }

   void pen_color(const unsigned char& red,
                  const unsigned char& green,
                  const unsigned char& blue)
   {
      pen_color_red_   = red;
      pen_color_green_ = green;
      pen_color_blue_  = blue;
   }

private:

   image_drawer(const image_drawer& id);
   image_drawer& operator =(const image_drawer& id);

   bitmap_image& image_;
   unsigned int  pen_width_;
   unsigned char pen_color_red_;
   unsigned char pen_color_green_;
   unsigned char pen_color_blue_;
};

#endif