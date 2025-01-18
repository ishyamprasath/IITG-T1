# IITG-T1
**Overview of the Project: Book Similarity Checker**

This project analyzes a collection of text files (representing books) to determine similarities based on word frequencies. The program identifies the top 10 most similar pairs of books from a collection of text files.



### Key Concepts

1. **Word Frequency:**
   - Determines the frequency of word occurrences in a given text file.
   - Similar texts are expected to have higher frequencies for similar words.

2. **Text Cleaning:**
   - Keeps only alphanumeric characters.
   - Converts all characters to uppercase for uniformity.

3. **Stop Words Removal:**
   - Common words like "a", "and", "an", "of", "in", and "the" are removed to focus on more significant vocabulary.

4. **Normalization:**
   - Word frequencies are normalized by dividing each frequency by the total word count, enabling comparison of files of different lengths.

5. **Similarity Index:**
   - Calculated based on the sum of normalized word frequencies of their top 100 most frequent words.

6. **Multi-Language Support:**
   - Designed to work with multiple languages.



### Step-by-Step Explanation of the Code

#### **Including Libraries**
```cpp
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
```
- **Standard Libraries Used:**
  - `iostream`: For input/output operations.
  - `fstream`: Handles file input/output.
  - `string`: Manages text strings.
  - `vector`: Stores data like words and frequencies.
  - `map`: Maintains word frequency counts.
  - `cctype`: Supports character manipulations (e.g., converting to uppercase).
  - `algorithm`: Used for sorting.
  - `sstream`: Enables string tokenization.
  - `filesystem`: Facilitates file and directory operations.

#### **Cleaning the Text**
```cpp
std::string clean(const std::string& text) {
    std::string cleanedText = "";
    for (char c : text) {
        if (std::isalnum(c)) {
            cleanedText += std::toupper(c);
        } else if (std::isspace(c)) {
            cleanedText += ' ';
        } else {
            cleanedText += ' ';
        }
    }
    return cleanedText;
}
```
- **Purpose:** Processes raw text for analysis.
- **Logic:**
  - Retains alphanumeric characters.
  - Converts all characters to uppercase.

#### **Calculating Word Frequencies**
```cpp
std::map<std::string, int> wordCounts(const std::string& text) {
    std::map<std::string, int> counts;
    std::istringstream iss(text);
    std::string word;
    std::vector<std::string> stopWords = {"A", "AND", "AN", "OF", "IN", "THE"};
    while (iss >> word) {
        bool isStopWord = false;
        for (const auto& stopWord : stopWords) {
            if (word == stopWord) {
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
```
- **Purpose:** Counts the frequency of each word, excluding stop words.
- **Logic:**
  - Tokenizes text using spaces.
  - Filters out predefined stop words.
  - Stores frequencies in a `map`.

#### **Normalizing Word Frequencies**
```cpp
std::map<std::string, double> normalizeFreqs(const std::map<std::string, int>& counts, int total) {
    std::map<std::string, double> freqs;
    for (const auto& pair : counts) {
        freqs[pair.first] = static_cast<double>(pair.second) / total;
    }
    return freqs;
}
```
- **Purpose:** Normalizes word frequencies.
- **Logic:**
  - Divides each word’s frequency by the total word count.

#### **Extracting Top N Most Frequent Words**
```cpp
std::vector<std::pair<std::string, double>> topWords(const std::map<std::string, double>& freqs, int n) {
    std::vector<std::pair<std::string, double>> sortedWords(freqs.begin(), freqs.end());
    std::sort(sortedWords.begin(), sortedWords.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    std::vector<std::pair<std::string, double>> topWords;
    for (int i = 0; i < std::min(n, static_cast<int>(sortedWords.size())); ++i) {
        topWords.push_back(sortedWords[i]);
    }
    return topWords;
}
```
- **Purpose:** Retrieves the top N frequent words.
- **Logic:**
  - Sorts words by normalized frequency.
  - Picks the top N words.

#### **Calculating Similarity Score**
```cpp
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
```
- **Purpose:** Computes similarity between two word frequency lists.
- **Logic:**
  - Adds normalized frequencies of matching words.

#### **Main Function**
```cpp
int main() {
    std::vector<std::string> allText;
    std::vector<std::string> bookNames;
    std::string folder = "./Book-Txt";

    if (!fs::exists(folder) || !fs::is_directory(folder)) {
        std::cerr << "Error: The path does not exist or is not a directory." << std::endl;
        return 1;
    }

    int bookCount = 0;
    for (const auto& entry : fs::directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            std::ifstream file(entry.path());
            std::string filename = entry.path().filename().string();
            bookNames.push_back(filename);
            if (file.is_open()) {
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

    if (bookCount != 64) {
        std::cerr << "Error parsing books. The number of books found is not 64, but " << bookCount << std::endl;
        return 1;
    }

    std::vector<std::vector<std::pair<std::string, double>>> allWords;
    for (const auto& text : allText) {
        auto counts = wordCounts(text);
        int total = 0;
        for (const auto& pair : counts) total += pair.second;
        auto freqs = normalizeFreqs(counts, total);
        allWords.push_back(topWords(freqs, 100));
    }

    std::vector<std::vector<double>> simMatrix(64, std::vector<double>(64, 0.0));
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
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

    return 0;
}
```
- **Purpose:**
  - Reads files from a specified folder.
  - Cleans, processes, and normalizes text.
  - Calculates similarity scores and outputs the top 10 most similar book pairs.
- **Error Handling:**
  - Ensures the correct number of books are processed.



### Sample Output
```plaintext
Top 10 Similar Book Pairs:
Pair 1: (PrideAndPrejudice.txt, SenseAndSensibility.txt) - Similarity: 0.123456
Pair 2: (MobyDick.txt, TheOldManAndTheSea.txt) - Similarity: 0.112345
Pair 3: (WarAndPeace.txt, AnnaKarenina.txt) - Similarity: 0.101234
Pair 4: (GreatExpectations.txt, DavidCopperfield.txt) - Similarity: 0.090123
Pair 5: (TaleOfTwoCities.txt, HardTimes.txt) - Similarity: 0.089012
Pair 6: (Emma.txt, Persuasion.txt) - Similarity: 0.078901
Pair 7: (JaneEyre.txt, WutheringHeights.txt) - Similarity: 0.067890
Pair 8: (TheAdventuresOfTomSawyer.txt, TheAdventuresOfHuckleberryFinn.txt) - Similarity: 0.056789
Pair 9: (Frankenstein.txt, Dracula.txt) - Similarity: 0.045678
Pair 10: (ThePictureOfDorianGray.txt, TheImportanceOfBeingEarnest.txt) - Similarity: 0.034567
```

---

**Done by - Shyam Prasath S**

**IITGCS_24091594**

