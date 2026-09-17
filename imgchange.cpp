#include <iostream>
#include <fstream>
#include <cmath> 
#include <string>

int main(int argc, char* argv[])
{

    if (argc != 5)
    {

       std::cout << "Usage: ./imgchange input.bmp output.bmp operation factor"
                 << std::endl;
       return 1;
    }
   std::string inputFile = argv[1];
   std::string outputFile = argv[2];
   std::string operation = argv[3];
   double factor = std::stod(argv[4]);

   std::ifstream file(inputFile, std::ios::binary);

  if (!file)
  {
 
    std::cout << "Could not open flowers.bmp" << std::endl;
    return 1;
  }

  unsigned char header[54];
  file.seekg(0);
  file.read(reinterpret_cast<char*>(header), 54);

  int width;
  int height;
  unsigned short bitsPerPixel;
  int pixelOffset;
  

  file.seekg(18);
  file.read(reinterpret_cast<char*>(&width), 4);
 
  file.seekg(22);
  file.read(reinterpret_cast<char*>(&height), 4);

  file.seekg(28);
  file.read(reinterpret_cast<char*>(&bitsPerPixel), 2);

  file.seekg(10);
  file.read(reinterpret_cast<char*>(&pixelOffset), 4);
 
  int rowSize = ((width * 3 + 3) /4) * 4;
  
  unsigned char* pixels = new unsigned char[rowSize * height];
  

  file.seekg(pixelOffset);

  file.read(reinterpret_cast<char*>(pixels), rowSize * height);
  
  for (int y = 0; y < height ; y++) 
  {
       for (int x = 0; x < width; x++)
       { 
            int index = y * rowSize + x * 3; 
            
            double blue = pixels[index] / 255.0;
            double green = pixels[index + 1] / 255.0;
            double red = pixels[index + 2] / 255.0;
    
            if (operation == "contrast")
            {
 
              blue = pow(blue, factor);
              green = pow(green, factor);
              red = pow(red, factor);
            }
 
            if (operation == "saturation")
            {
               double average = (red + green + blue) / 3.0;
               red = red + (red-average) * factor;
               green = green + (green - average) * factor;
               blue = blue + (blue - average) * factor;
            }
    
            if (operation == "lightness")
            {
               red = red + factor;
               green = green + factor;
               blue = blue + factor;
            }
            
            if (red < 0) red = 0;
            if (red > 1) red = 1;
           
            if (green < 0) green = 0;
            if (green > 1) green = 1;
        
            if (blue < 0) blue = 0;
            if (blue > 1) blue = 1;


            pixels[index] = (unsigned char)(blue * 255); 
            pixels[index + 1] = (unsigned char)(green *255);
            pixels[index + 2] = (unsigned char)(red * 255);
            
           
       }
  }


  std::cout << "Width: " << width << std::endl;
  std::cout << "Height: " << height << std::endl;
  std::cout << "Bits per pixel: " << bitsPerPixel << std:: endl;
  std::cout << "Pixel data starts at: " << pixelOffset << std::endl;
  std::cout << "Row size: " << rowSize << std::endl; 

  std::ofstream output(outputFile, std::ios::binary);

  if (!output)
  {

     std::cout << "Could not create" << outputFile << std::endl;

     delete[] pixels;
     file.close();

     return 1;
  }

  output.write(
      reinterpret_cast<char*>(header),
      54
  );
 
  output.write(
      reinterpret_cast<char*>(pixels),
      rowSize * height 
  );

  output.close();
  file.close();
 
  delete[] pixels;
 
  return 0;
  }


  
  