#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>
#include <vector>
#include <map>
#include <string.h>

template <typename T>
void getData(std::fstream &f,T &v)
{
    char* buffer = new char[sizeof(T)];
    f.read(buffer, sizeof(T));
    v = *(T*)buffer;
}

int ReadBit6(std::fstream &file,int byteCount)
{
    unsigned int result = 0;
    unsigned int shift = 0;
    char* b = new char[1];
    unsigned int i = 1;
    do
    {
        b[0] = 0;
        file.read(b,1);
        //std::cout<<"b: " << (unsigned int)b[0] << " " << (unsigned int)(b[0] & 0x80) << ", ";
        if ((unsigned int)b[0] == 128)
            return 0;
        unsigned int s = 6;
        unsigned int mask = 255;
        if ((unsigned int)b[0] > 127)
        {
            mask = 127;
            s = 7;
        }
        else if ((unsigned int)b[0] > 63)
        {
            if (i == 1)
            {
                mask = 63;
            }
        }
        result = result | ((b[0] & mask) << shift);
        shift = shift + s;
        i = i + 1;
    } while(i < byteCount + 1);
    //std::cout<<std::endl;
    return result;
}

int ReadBit6(std::fstream &file)
{
    unsigned int result = 0;
    unsigned int shift = 0;
    char* b = new char[1];
    unsigned int i = 1;
    do
    {
        b[0] = 0;
        file.read(b,1);
        //std::cout<<"b: " << (unsigned int)b[0] << " " << (unsigned int)(b[0] & 0x80) << ", ";
        if ((unsigned int)b[0] == 128)
            return 0;
        unsigned int s = 6;
        unsigned int mask = 255;
        if ((unsigned int)b[0] > 127)
        {
            mask = 127;
            s = 7;
        }
        else if ((unsigned int)b[0] > 63)
        {
            if (i == 1)
            {
                mask = 63;
            }
        }
        result = result | ((b[0] & mask) << shift);
        shift = shift + s;
        i = i + 1;
    } while (!((unsigned int)b[0] < 64 || (i >= 3 && (unsigned int)b[0] < 128)));
    //std::cout<<std::endl;
    return result;
}

class SpeechElement
{
    public:
        uint32_t id;
        uint32_t high_id;
        uint32_t offset;
        uint32_t size;
    SpeechElement(std::fstream &f)
    {
        uint32_t garbage;
        getData(f,this->id);
        getData(f,this->high_id);
        getData(f,this->offset);
        this->offset += 4; 
        getData(f,garbage);
        getData(f,this->size);
        this->size -= 12;
        getData(f,garbage);
        getData(f,garbage);
        getData(f,garbage);
        getData(f,garbage);
        getData(f,garbage);
    }
    SpeechElement()
    {
        this->id = 0;
        this->high_id = 0;
        this->offset = 0;
        this->size = 0;
    }
    void debugPrint()
    {
        std::cout << "SpeechElement: " << std::hex << this->id << " " << this->high_id << std::dec << " " << this->offset << " " << this->size << std::endl;
    }
};

int getLength(std::fstream &file,int fsize)
{
    bool isHeaderValid = false;
    int checkPasses = 1;
    int bitLen = 1;
    int fileCount = 0;
    int bitLimit = 20;

    while(!isHeaderValid && bitLen < bitLimit)
    {
        file.seekg(10);//return to first filecount bit
        //std::cout << bitLen << std::endl;
        fileCount = ReadBit6(file,bitLen);
        //single info section = 40 bits
        for(int i=0;i<checkPasses;i++)
        {
            SpeechElement info = SpeechElement(file);//get info
            if(info.offset >= fsize)//check if out of bounds
            {
                //std::cout << "out of bounds" << std::endl;
                isHeaderValid = false;
                bitLen ++;
                break;
            }
            file.seekg(info.offset);
            char* riffHeader = new char[4];//read wav header
            file.read(riffHeader,4);
            //std::cout<< riffHeader << " " << info.offset << " " << fileCount << std::endl;
            if(strcmp(riffHeader,"RIFF") != 0)//check 4 bits of wav if false isHeaderValid=false and break
            {
                isHeaderValid = false;
                bitLen ++;
                break;
            }
            file.seekg(10+bitLen+40*i);
            isHeaderValid = true;
        }
    }
    file.seekg(10 + bitLen);//offset file to info section location
    return fileCount;
}

std::map<std::string, uint32_t> keydict = {{"pl",0x73946816U,},{"en",0x79321793U,},{"br",0x00000000U,},{"ru",0x42386347U,},{"jp",0x59825646U,},{"fr",0x75921975U,},{"de",0x42791159U,}};

int main(int argc, char **argv)
{
    //no file
    if(argc != 4)
    { 
        std::cout << "Usage: \n\tscript file language output_directory." << std::endl; 
        return 0;
    }
    std::cout << "File:" << argv[1] << std::endl;
    //file specified
    //open file
    std::fstream file;
    std::string lang = argv[2];
    std::string output_directory = argv[3];
    file.open(argv[1],std::fstream::binary | std::fstream::in | std::fstream::ate);
    if (!file)
    {
        std::cout << "couldnt read file" << std::endl;
        return 0;
    }
    //get file size
    int fileSizeInBytes = file.tellg();
    //open again becouse ate flag breaks the code
    file.close();
    file.open(argv[1],std::fstream::binary | std::fstream::in);
    //read header
    char* str = new char[4];
    file.read(str,4);
    uint32_t version;
    getData(file,version);
    uint16_t key1;
    getData(file,key1);
    
    //item count
    //unsigned int itemcount = ReadBit6(file);
    unsigned int itemcount = getLength(file,fileSizeInBytes);
    std::cout << "item count: " << itemcount << std::endl;
    //get wavs
    std::vector<SpeechElement> speechInfo;
    for(int i=0;i<itemcount;i++)
    {
        speechInfo.push_back(SpeechElement(file));
    }
    //save file
    std::stringstream stream;
    for(int i=0;i<speechInfo.size();i++)
    {
        //get data
        char* wav = new char[speechInfo[i].size];
        file.seekg(speechInfo[i].offset);
        file.read(wav,speechInfo[i].size);
        //get name
        //speechInfo[i].debugPrint();
        stream << "0x" << std::setfill('0') << std::setw(sizeof(uint32_t)*2) << std::hex << (uint32_t)(speechInfo[i].id ^ keydict[lang]) << ".wav";
        std::string name = stream.str();
        stream.str(std::string());
        std::cout<<name<<std::endl;
        stream << output_directory << "/" << name;
        std::string filepath = stream.str();
        //write to file
        std::ofstream out(filepath,std::ios::out | std::ios::binary);
        out.write(wav,speechInfo[i].size);
        out.close();
        stream.str(std::string());
    }
    file.close();
}
