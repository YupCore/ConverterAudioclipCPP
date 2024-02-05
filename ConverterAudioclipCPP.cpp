#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <sstream>
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

template <typename T>
std::vector<T> StringToBytesArray(const std::string& str) {
    int bitsToPad = 8 - str.length() % 8;

    if (bitsToPad != 8) {
        int neededLength = bitsToPad + str.length();
        std::string paddedStr = std::string(neededLength, '0') + str;
        return std::vector<T>(paddedStr.begin(), paddedStr.end());
    }

    int size = str.length() / 8;
    std::vector<T> arr(size);

    for (int a = 0; a < size; a++) {
        arr[a] = static_cast<T>(std::bitset<8>(str.substr(a * 8, 8)).to_ulong());
    }

    return arr;
}

template <typename T>
void WriteBytesToFile(const std::string& filePath, const std::vector<T>& data) {
    std::ofstream outputFile(filePath, std::ios::out | std::ios::binary);
    outputFile.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(T));
    outputFile.close();
}

void ProcessFile(const std::string& filePath) {
    std::ifstream inputFile(filePath);
    std::string fileContent((std::istreambuf_iterator<char>(inputFile)),
        std::istreambuf_iterator<char>());

    size_t startId = fileContent.find("m_AudioData:") + 13;
    size_t endId = fileContent.find("m_EditorAudioData:") - 4;

    std::string hexData = fileContent.substr(startId, endId - startId + 1);

    std::string binaryString;
    std::stringstream ss;

    for (char c : hexData) {
        ss << std::hex << c;
        int intValue;
        ss >> intValue;
        binaryString += std::bitset<4>(intValue).to_string();
        ss.clear();
    }

    std::vector<unsigned char> byteArray = StringToBytesArray<unsigned char>(binaryString);

    std::string newFilePath = filePath;
    newFilePath.replace(filePath.find_last_of("."), std::string::npos, ".wav");
    WriteBytesToFile(newFilePath, byteArray);
}

int main() {
    std::cout << "Input folder path:" << std::endl;
    std::string folderPath;
    std::cin >> folderPath;

    std::vector<std::thread> threads;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string filePath = entry.path().string();
            threads.emplace_back(ProcessFile, filePath);
        }
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Done, press any key..." << std::endl;
    std::cin.ignore();
    std::cin.get();

    return 0;
}