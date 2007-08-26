/*
 **************************************************************************
 *                                                                        *
 *                         Platform Independent                           *
 *                   Bitmap Image Reader Writer Library                   *
 *                                                                        *
 * Author: Arash Partow - 2002                                            *
 * URL: http://www.partow.net                                             *
 *                                                                        *
 * Copyright notice:                                                      *
 * Free use of the Platform Independent Bitmap Image Reader Writer Library*
 * is permitted under the guidelines and in accordance with the most      *
 * current version of the Common Public License.                          *
 * http://www.opensource.org/licenses/cpl.php                             *
 *                                                                        *
 **************************************************************************
*/


#include <string>
#include "bitmap_image.hpp"

int main(void)
{
   std::string file_name = "image.bmp";
   bitmap_image image("image.bmp");
   image.save_image("saved.bmp");
   image.vertical_flip();
   image.save_image("saved_vert_flip.bmp");
   image.vertical_flip();
   image.horizontal_flip();
   image.save_image("saved_horiz_flip.bmp");
   return 0;
}
