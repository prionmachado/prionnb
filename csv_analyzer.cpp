#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <thread>
#include <algorithm>
#include <numeric>

// Function to parse a CSV line
void parseCSVLine(const std::string& line, std::vector<double>& numericValues) {
    std::stringstream ss(line);
    std::string cell;
    while (std::getline(ss, cell, ',')) {
        try {
            numericValues.push_back(std::stod(cell)); // Convert to double if possible
        } catch (...) {
            numericValues.push_back(0.0); // Placeholder for non-numeric values
        }
    }
}

// Compute sum and mean for given columns in parallel
void computeSumAndMean(const std::vector<std::vector<double>>& data, std::vector<double>& sum, std::vector<double>& mean, size_t start, size_t end) {
    for (size_t j = start; j < end; ++j) {
        sum[j] = std::accumulate(data.begin(), data.end(), 0.0, 
                                 [&](double acc, const std::vector<double>& row) { return acc + row[j]; });
        mean[j] = sum[j] / data.size();
    }
}

// Compute median for given columns in parallel
void computeMedian(std::vector<std::vector<double>>& data, std::vector<double>& median, size_t start, size_t end) {
    for (size_t j = start; j < end; ++j) {
        std::vector<double> column;
        for (auto& row : data) column.push_back(row[j]);

        std::nth_element(column.begin(), column.begin() + column.size() / 2, column.end());
        median[j] = column[column.size() / 2];

        if (column.size() % 2 == 0) {
            std::nth_element(column.begin(), column.begin() + (column.size() / 2) - 1, column.end());
            median[j] = (median[j] + column[(column.size() / 2) - 1]) / 2.0;
        }
    }
}

// Function to process the CSV file
void processCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file!\n";
        return;
    }

    std::string line;
    std::vector<std::vector<double>> numericData;

    while (std::getline(file, line)) {
        std::vector<double> numericValues;
        parseCSVLine(line, numericValues);
        numericData.push_back(numericValues);
    }
    file.close();
    
    if (numericData.empty()) {
        std::cerr << "No valid data found!\n";
        return;
    }
    
    size_t cols = numericData[0].size();
    std::vector<double> sum(cols, 0.0), mean(cols, 0.0), median(cols, 0.0);
    
    size_t num_threads = std::min(std::thread::hardware_concurrency(), static_cast<unsigned int>(cols));
    std::vector<std::thread> threads;
    size_t chunk_size = cols / num_threads;
    
    // Parallel computation of sum & mean
    for (size_t t = 0; t < num_threads; ++t) {
        size_t start = t * chunk_size;
        size_t end = (t == num_threads - 1) ? cols : start + chunk_size;
        threads.emplace_back(computeSumAndMean, std::cref(numericData), std::ref(sum), std::ref(mean), start, end);
    }
    for (auto& th : threads) th.join();
    threads.clear();
    
    // Parallel computation of median
    for (size_t t = 0; t < num_threads; ++t) {
        size_t start = t * chunk_size;
        size_t end = (t == num_threads - 1) ? cols : start + chunk_size;
        threads.emplace_back(computeMedian, std::ref(numericData), std::ref(median), start, end);
    }
    for (auto& th : threads) th.join();
    
    // Display results
    std::cout << "Column Statistics:\n";
    for (size_t j = 0; j < cols; ++j) {
        std::cout << "Column " << j + 1 << ": Sum = " << sum[j] 
                  << ", Mean = " << mean[j] 
                  << ", Median = " << median[j] << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <csv_file>\n";
        return 1;
    }
    
    processCSV(argv[1]);
    return 0;
}