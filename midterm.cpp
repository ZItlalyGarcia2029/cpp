#include <iostream> 
#include <fstream>
#include <vector> 
#include <chrono>
#include <cstdint>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>



using namespace std;

//file header

int main(int argc, char** argv)
{
    string fileAName = "imageA.bmp";
    string fileBName = "imageB.bmp";
//processing
    int n = 4;
//blending
    double t = 0.5;
//open images 
    ifstream fileA(fileAName, ios::binary);
    ifstream fileB(fileBName, ios::binary);
//debugger
    if(!fileA || !fileB)
    {
       cout << "Could not open images" << endl;
       return 1;
    }

//cannot hold negative numbers/bit width #  
    uint32_t offset;

    fileA.seekg(10);
    fileA.read((char*)&offset, 4);


    int width;
    fileA.seekg(18);
    fileA.read((char*)&width,4);

    int height;
    fileA.seekg(22);
    fileA.read((char*)&height, 4);
//calculate row 
    int rowSize = ((width * 3 + 3) /4)*4;

    int imageSize = rowSize * height;

    vector<unsigned char> A(imageSize);

    fileA.seekg(offset);
    fileA.read((char*)A.data(), imageSize);
   
    vector<unsigned char> B(imageSize);

    fileB.seekg(offset);
    fileB.read((char*)B.data(), imageSize);

//shared memory 
    unsigned char* result = 
      (unsigned char*)mmap(
          nullptr,
          imageSize, 
          PROT_READ | PROT_WRITE,
          MAP_SHARED | MAP_ANONYMOUS,
          -1, 
         0


       );
    if(result == MAP_FAILED)
 
    {
      cout << "mmap failed" << endl;
      return 1;

    }
//timer 
    auto start = chrono::high_resolution_clock::now();
    int rowsPerProcess = height / n;
    for (int i = 0; i < n; i++)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
          cout << "fork failed" << endl;
          munmap(result, imageSize);

          return 1;
        } 
        if (pid == 0)
        {
            int startRow = i * rowsPerProcess;
            int endRow;
            if (i == n - 1)
            {
                endRow = height;
            }
            else

            {
                endRow = startRow + rowsPerProcess;
            }
//assigned rows 

            for (int y = startRow; y < endRow; y++)
            {
 
                for (int x = 0; x < rowSize; x++)
                {
                    int index = y * rowSize + x;
                    result[index] = 
                         A[index] * t + 
                         B[index] * (1 - t);
                }
            }
            exit(0);

        }
    }
    for (int i = 0; i < n; i++)
    { 
       wait(nullptr);
    } 

    auto end = chrono::high_resolution_clock::now();
    
    auto elapsed = chrono::duration_cast<chrono::microseconds>(end - start);

    cout << "Processing Time:" 
         << elapsed.count()
         << "microseconds"
         << endl;
     
    ofstream output("result.bmp", ios::binary);

    if (!output)
    {
     cout << "Output file denied" << endl;
     munmap(result, imageSize);


     return 1;

    }
//bmp header restate
    fileA.clear();


    fileA.seekg(0);

    vector<char> header(offset);
    fileA.read(header.data(), offset);
 
    output.write(header.data(), offset);
    output.write((char*)result, imageSize);

    output.close();

    munmap(result, imageSize);

    cout << "result.bmp created" << endl;
    return 0;

}
   
  





















        
    


  








    
  
 
    
 
 






