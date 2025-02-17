#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <list>
#include <iterator>
#include <algorithm>
#include <cmath>
#include <vector>
#include <tuple>
#include <random>
#include <unistd.h>
#include <cstdlib>
#include <chrono>


using namespace std;

///////////////////////////////////////////////////////

void read_file();

void calculate_probabilities();

void print_unordered_map();

void predict();

void write_to_file();

/////////////// global variables //////////////////////

static unordered_map<string, vector<float> > un_map;
static string file_name;
static ifstream file;
static vector<char> different_symbols;
static int N_different_symbols = 0;

static vector<string> k_word_read_vector;
static int K = 3;
static int N = 500;
std::random_device rd;
std::mt19937 gen(rd());


///////////////////////////////////////////////////////

int main(int argc, char **argv)
{
     int opt;
    while ((opt = getopt(argc, argv, "f:n:k:")) != -1) {
        switch (opt) {
            case 'f':
                file_name = optarg;
                break;
            case 'n':
                N = atoi(optarg);
                break;
            case 'k':
                K = atoi(optarg);
                break;
            default:
                cout << "Usage: " << argv[0] << " -f <file_name> -n <number_of_chars_to_predict> -k <number_of_chars_to_read>" << endl;
                exit(1);
        }
    }

    read_file();
    calculate_probabilities();
    for (int i = 0; i <= N; i++)
    {
        predict();

    }
    //print_unordered_map();
    write_to_file();

    return 0;
}

void read_file()
{
    cout << "Reading file: " << file_name << endl;
    file.open(file_name);
    if (!file.is_open())
    {
        cout << "Error opening file" << endl;
        exit(1);
    }
    // read first K characters and then read one character at a time
    string word;
    for (int i = 0; i < K; i++)
    {
        char c;
        file.get(c);
        cout << "c: " << c << endl;
        if (find(different_symbols.begin(), different_symbols.end(), c) == different_symbols.end())
        {
            different_symbols.push_back(c);
            N_different_symbols++;
        }
        word += c;

    }
    k_word_read_vector.push_back(word);
    
    while (file.peek() != EOF)
    {
        char c;
        file.get(c);
        cout << "c: " << c << endl;
        if (find(different_symbols.begin(), different_symbols.end(), c) == different_symbols.end())
        {
            different_symbols.push_back(c);
            N_different_symbols++;
        }
        word += c;
        word.erase(0, 1);
        k_word_read_vector.push_back(word);
    }
    file.close();

}

void calculate_probabilities()
{
    for (int i = 0; i < k_word_read_vector.size(); i++)
    {
        string word = k_word_read_vector[i];
        cout << word << endl;

        //create a new entry in the map if the word is not in the map
        if (un_map.find(word) == un_map.end())
        {
            un_map[word] = vector<float>(N_different_symbols, 0);
        }

        if (i == k_word_read_vector.size() - 1)
        {
            break;
        }

        char next_char = k_word_read_vector[i + 1][K - 1];
        cout << next_char << endl;
        int index = find(different_symbols.begin(), different_symbols.end(), next_char) - different_symbols.begin();
        un_map[word][index]++;

        
    }

}

void print_unordered_map()
{
    cout << "unorderer_map" << endl;
    for (auto it = un_map.begin(); it != un_map.end(); it++)
    {
        cout << it->first << " ";
        for (int i = 0; i < it->second.size(); i++)
        {
            cout << it->second[i] << " ";
        }
        cout << endl;
    }
}

void predict()
{
    bool all_zero = true;
    // predict next char of the file based on the probabilities calculated

    //get the last K characters of the file
    
    string word = k_word_read_vector[k_word_read_vector.size() - 1];
    
    //cout << "old word: " << word << endl;
   
    //get the probabilities of each symbol for the last K characters
    vector<float> probabilities = un_map[word];
    float sum = 0;
    for (int i = 0; i < probabilities.size(); i++)
    {
        sum += probabilities[i];
    }
   
    //divide each probability by the sum
    for (int i = 0; i < probabilities.size(); i++)
    {
        probabilities[i] = probabilities[i] / sum;
        // if its Nan then set it to 0
        if (probabilities[i] != probabilities[i])
        {
            probabilities[i] = 0;
        }
        if (probabilities[i] != 0)
        {
            all_zero = false;
        }

    }
    //check if all probabilities are 0
    if (all_zero)
    {
        // if all probabilities are 0 then set them all to 1/N
        for (int i = 0; i < probabilities.size(); i++)
        {
            probabilities[i] = 1.0 / N_different_symbols;
        }
    }

    cout<< "word: " << word << endl;
    cout << "sum: " << sum << endl;

    cout << "probabilities: " << endl;
    for (int i = 0; i < probabilities.size(); i++)
    {
        cout << probabilities[i] << " ";
    }
    cout << endl;
    
    //pick a letter random using the probabilities calculated
    std::discrete_distribution<> d(probabilities.begin(), probabilities.end());

    int letter = d(gen);
    cout << "letter: " << letter << endl;
    
    char next_char = different_symbols[letter];
    cout << "next char: " << next_char << endl;
    //print all characters
    for (int i = 0; i < different_symbols.size(); i++)
    {
        cout << different_symbols[i] << " ";
    }
    
    // update the probabilities of the last K characters
    un_map[word][letter]++;
    
    
    //add the new letter to the vector 
    word.erase(0, 1);
    word += next_char;
    //cout << "new word: " << word << endl;
    k_word_read_vector.push_back(word);
    
    

    if (un_map.find(word) == un_map.end())
    {
        un_map[word] = vector<float>(N_different_symbols, 0);
    }
 
    
}

void write_to_file()
{
    ofstream file;
    file.open("output.txt");
    if (!file.is_open())
    {
        cout << "Error opening file" << endl;
        exit(1);
    }
    //write the first K characters of the file
    file << k_word_read_vector[0];

    // write the rest of the characters char by char
    for (int i = 1; i < k_word_read_vector.size(); i++)
    {
        file << k_word_read_vector[i][K - 1];
    }
    file.close();
}