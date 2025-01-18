#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

std::string clean(const std::string& text);
std::map<std::string, int> wordCounts(const std::string& text);
std::map<std::string, double> normalizeFreqs(const std::map<std::string, int>& counts, int total);
std::vector<std::pair<std::string, double>> topWords(const std::map<std::string, double>& freqs, int n = 100);
double similarityScore(const std::vector<std::pair<std::string, double>>& words1, const std::vector<std::pair<std::string, double>>& words2);


int main() {
    std::vector<std::string> allText;
    std::vector<std::string> bookNames;
    std::string folder = "./Book-Txt";

    if (!fs::exists(folder) || !fs::is_directory(folder)) {
        std::cerr << "Error: The path does not exist or is not a directory." << std::endl;
        return 1;
    }

    int bookCount = 0;
    for(const auto & entry : fs::directory_iterator(folder)) {
        if(entry.is_regular_file()) {
            std::ifstream file(entry.path());
             std::string filename = entry.path().filename().string();
           bookNames.push_back(filename);
            if(file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                allText.push_back(clean(content));
                file.close();
                bookCount++;
            } else {
                std::cerr << "Unable to open file: " << entry.path() << std::endl;
                return 1;
            }

        }
    }
      if(bookCount != 64){
        std::cerr << "Error parsing books. The number of books found is not 64, but " << bookCount << std::endl;
        return 1;
    }

    std::vector<std::vector<std::pair<std::string, double>>> allWords;
    for (const auto& text : allText) {
         auto counts = wordCounts(text);
        int total = 0;
        for(const auto& pair : counts) total+= pair.second;
        auto freqs = normalizeFreqs(counts, total);
        allWords.push_back(topWords(freqs));
    }

    std::vector<std::vector<double>> simMatrix(64, std::vector<double>(64, 0.0));
    for(int i=0; i < 64; i++)
    {
        for(int j=0; j < 64; j++)
        {
             if (i != j) {
                simMatrix[i][j] = similarityScore(allWords[i], allWords[j]);
            }
        }
    }


    std::vector<std::tuple<int, int, double>> simPairs;
    for (int i = 0; i < 64; ++i) {
        for (int j = i + 1; j < 64; ++j) {
            simPairs.emplace_back(i, j, simMatrix[i][j]);
        }
    }
    std::sort(simPairs.begin(), simPairs.end(), [](const auto& a, const auto& b) {
         return std::get<2>(a) > std::get<2>(b);
    });

    std::cout << "Top 10 Similar Book Pairs:\n";
     for (int i = 0; i < 10 && i < simPairs.size(); ++i) {
        int book1 = std::get<0>(simPairs[i]);
        int book2 = std::get<1>(simPairs[i]);
        double sim = std::get<2>(simPairs[i]);
         std::cout << "Pair " << i + 1 << ": (" << bookNames[book1] << ", " << bookNames[book2] << ") - Similarity: " << std::fixed << std::setprecision(6) << sim << std::endl;
    }

     std::cin.get();
    return 0;
}

std::string clean(const std::string& text) {
    std::string cleanedText = "";
    for (char c : text) {
        if (std::isalnum(c)) {
            cleanedText += std::toupper(c);
        } else if (std::isspace(c)){
          cleanedText += ' ';
         } else {
            cleanedText+= ' ';
        }
    }
    return cleanedText;
}

std::map<std::string, int> wordCounts(const std::string& text) {
    std::map<std::string, int> counts;
    std::istringstream iss(text);
    std::string word;
     std::vector<std::string> stopWords = {"A", "AND", "AN", "OF", "IN", "THE"};
    while (iss >> word) {
        bool isStopWord = false;
        for(const auto& stopWord : stopWords) {
            if (word == stopWord){
                isStopWord = true;
                break;
            }
        }
        if (!isStopWord) {
           counts[word]++;
        }
    }
    return counts;
}

std::map<std::string, double> normalizeFreqs(const std::map<std::string, int>& counts, int total) {
    std::map<std::string, double> freqs;
    for (const auto& pair : counts) {
        freqs[pair.first] = static_cast<double>(pair.second) / total;
    }
    return freqs;
}

std::vector<std::pair<std::string, double>> topWords(const std::map<std::string, double>& freqs, int n) {
     std::vector<std::pair<std::string, double>> sortedWords(freqs.begin(), freqs.end());
    std::sort(sortedWords.begin(), sortedWords.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    std::vector<std::pair<std::string, double>> topWords;
    for (int i = 0; i < std::min(n, (int)sortedWords.size()); ++i) {
        topWords.push_back(sortedWords[i]);
    }
    return topWords;
}

double similarityScore(const std::vector<std::pair<std::string, double>>& words1, const std::vector<std::pair<std::string, double>>& words2) {
    double sim = 0.0;
    for (const auto& word1 : words1) {
        for (const auto& word2 : words2) {
            if (word1.first == word2.first) {
                sim += word1.second + word2.second;
            }
        }
    }
    return sim;
}