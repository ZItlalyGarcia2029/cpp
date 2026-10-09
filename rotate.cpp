#include <iostream>  
#include <fstream>
#include <cmath>
#include <cstring> 
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>



using namespace std;


int main(int argc, char* argv[])
{
  if (argc != 5) 

{
    cout << "Usage: ./rotate[IMAGEFILE} [ANGLE] [N PROCESSES] [OUTPUTFILE]" << endl;
    return 1;
}

    // hardcoded values/temporary settings
   string inputFile = argv[1];
   double angle = stod(argv[2]); 
   int numProcesses = stoi(argv[3]);

   if (numProcesses < 1 || numProcesses > 4)
   {

      cout << "Number of processes must be between 1 and 4. " << endl;
      return 1;
   }
 
   string outputFile = argv[4];
    
   double radians = angle;

   //for now only one process
   //this means to open lion.bmp so we can read its binary data 
   //ios::binary means treat the file as binary data instead of text 
   ifstream file(inputFile, ios::binary);
   if (!file)
   {
     cout << "Could not open image file." << endl;
     return 1;
   }
   
   cout << "Image opened successfully!" << endl;
   
   //variable for bmp information
   int width;
   int height;
   int pixelOffset;
   short bitsPerPixel;

   //Read width 
   file.seekg(18);
   file.read(reinterpret_cast<char*>(&width), sizeof(width));

   //Read height 
   file.seekg(22);
   file.read(reinterpret_cast<char*>(&height), sizeof(height));

   //moves to byte 10, read Pixel data offset 
   file.seekg(10);
   //read that value into..int pixelOffset
   file.read(reinterpret_cast<char*>(&pixelOffset), sizeof(pixelOffset));

  // read bits per pixel 
   file.seekg(28);
   file.read(reinterpret_cast<char*>(&bitsPerPixel), sizeof(bitsPerPixel));


   cout << "Width:" << width << endl;
   cout << "Height:" << height << endl;
   cout << "Pixel offset:" << pixelOffset << endl;
   cout << "Bits per pixel:" << bitsPerPixel << endl;
   cout << "Input file:" << inputFile << endl;
   cout << "Output file:" << outputFile << endl;
   cout << "Angle:" << angle << endl;
   cout << "Processes:" << numProcesses << endl;

   int rowSize = ((width * bitsPerPixel +31) / 32) *4;
   
   cout << "Row size:" << rowSize << endl;
   
   int imageSize = rowSize * height;
   cout << "Image size:" << imageSize << " bytes" << endl;

   //point to our pixel memory 
  // mmap(..) means ask the operating system to give us a region of memory 
   unsigned char* pixels = (unsigned char*) mmap(
      nullptr,
      imageSize,
   // these two mean Read --> look at pixels 
   // Write --> change pixels 
      PROT_READ | PROT_WRITE,
      MAP_SHARED | MAP_ANONYMOUS, 
      -1, 
      0
   );

   if (pixels == MAP_FAILED)
   {

    cout << "mmap failed." << endl;
    return 1;
   }
  
   cout << "Shared memory allocated successfully!" << endl;

   unsigned char* outputPixels = (unsigned char*) mmap(
       nullptr,
       imageSize,
       PROT_READ | PROT_WRITE,
       MAP_SHARED | MAP_ANONYMOUS,
       -1, 
       0
   );

   if (outputPixels == MAP_FAILED)
  {
     cout << "Output mmap failed." << endl;
     return 1;

  }

  cout << "Output shared memory allocated successfully!" << endl;

   file.seekg(pixelOffset);
  
   file.read(reinterpret_cast<char*>(pixels), imageSize);

   if (!file)
  {
     cout << "Could not read pixel data." << endl;
     return 1;

  }
  cout << "Pixel data loaded into shared memory!" << endl;

 
  double centerX = width / 2.0;
  double centerY = height / 2.0;
  
  cout << "Center: (" << centerX << " , " << centerY << ")" << endl;


  auto startTime = chrono::high_resolution_clock::now();

  for (int process = 0; process < numProcesses; process++)
  {
      pid_t pid = fork();

      if (pid == -1)
      {

          cout << "Fork failed." << endl;
          return 1;
       }
       if (pid == 0)
       {
          int startRow = process * height / numProcesses; 
          int endRow = (process + 1) * height / numProcesses; 

         for (int y = startRow; y < endRow; y++)
        {
            for (int x = 0; x < width; x++)
           {

              double xr = x - centerX;
              double yr = y - centerY;

              double xot = cos(-radians) * xr - sin(-radians) * yr;
              double yot = sin(-radians) * xr + cos(-radians) * yr;

              double xo = xot + centerX;
              double yo = yot + centerY;

              int sourceX = static_cast<int>(round(xo));
              int sourceY = static_cast<int>(round(yo));

             if (sourceX < 0 || sourceX >= width ||
                sourceY < 0 || sourceY >= height) 
            {
              //outside image --> black
              int outputRow = height - 1 - y;
    
              int outputIndex = outputRow * rowSize + x * 3;
 
              outputPixels[outputIndex] = 0;
              outputPixels[outputIndex + 1] = 0;
              outputPixels[outputIndex + 2] = 0;
           }
           else
           {              //inside iamge --> get original pixel 
             int sourceRow = height - 1 - sourceY;
 
             int sourceIndex = sourceRow * rowSize + sourceX * 3;

             unsigned char blue = pixels[sourceIndex];
             unsigned char green = pixels[sourceIndex +1];
             unsigned char red = pixels[sourceIndex +2];

             int outputRow = height - 1 - y;
             
             int outputIndex = outputRow * rowSize + x * 3;

             outputPixels[outputIndex] = blue;
             outputPixels[outputIndex + 1] = green;
             outputPixels[outputIndex + 2] = red;

           }
       }

    }
    
    return 0;
  }

}

for (int process = 0; process < numProcesses; process++)
{
     wait(nullptr);
}
 
auto endTime = chrono::high_resolution_clock::now();

chrono::duration<double> elapsed = endTime - startTime;

cout << "Rotation time:" << elapsed.count() << "seconds" << endl;

ofstream output(outputFile, ios::binary);
   
if (!output)
{

    cout << "Could not create output file." << endl;
    return 1;
}


file.seekg(0);

char*header = new char[pixelOffset];

file.read(header, pixelOffset);

output.write(header, pixelOffset);

delete[] header;

output.write(reinterpret_cast<char*>(outputPixels), imageSize);

output.close(); 

cout << "Rotated image saved to " << outputFile << endl;

munmap(pixels, imageSize);
munmap(outputPixels, imageSize);

return 0;

}