#include "qb/core/benchmark/DataLoaderGenerator.hpp"

namespace qb {
    namespace benchmark {

        std::unordered_map<std::time_t, std::vector<std::string>> DataLoaderGenerator::loadAvailableTimestamps() const {
            //find and collect all timestamps for same workflow identifiers
            std::unordered_map<std::time_t, std::vector<std::string>> mTime2Files;
            for ( const auto& entry : std::filesystem::directory_iterator(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME) ) {
                std::string file = entry.path().filename().string();
                if ( file.substr(0, workflow_identifier_.size()) == workflow_identifier_ ) 
                {
                    std::regex r("[a-zA-Z]+_[a-zA-Z]+_([0-9]+)\\.bin"); 
                    std::smatch match; 
                    if ( std::regex_match(file, match, r) ) {
                        std::stringstream ss(match[1].str());
                        std::time_t t; 
                        ss >> t; 
                        if ( mTime2Files.insert(std::make_pair(t, std::vector<std::string>({file}))).second == false ) {
                            mTime2Files[t].push_back(file);
                        }
                    }
                } 
            }
            return mTime2Files;
        }


        std::vector<std::time_t> DataLoaderGenerator::filterTimestamps(const std::unordered_map<std::time_t, std::vector<std::string>>& available_timestamps) const {
            //remove all timestamp entries that do not have all metric identifiers aka match to the stored metric_regex
            std::vector<std::time_t> matching_timestamps;
            for ( auto& [timestamp, filenames] : available_timestamps )
            {
                size_t match_count = 0;
                for ( const std::string& filename : filenames  )
                    for ( const std::regex& r : metric_regex_ )
                    {
                        std::smatch temp;
                        if ( std::regex_match(filename, temp, r ) )
                            ++match_count; 
                    }
                if ( match_count == metric_regex_.size() )
                    matching_timestamps.push_back(timestamp);
            }

            return matching_timestamps;
        }

        bool DataLoaderGenerator::processUserInput(const std::vector<std::time_t>& matching_timestamps) {
            if ( matching_timestamps.size() > 0 ) {
                std::cout << "Found " << matching_timestamps.size() << " stored result(s) of " << workflow_identifier_ << " workflows:" << std::endl;
                for ( size_t i = 0; i < matching_timestamps.size(); ++i )
                    std::cout << "[" << i << "] -- " << "UTC: " << std::put_time(std::gmtime(&matching_timestamps[i]), "%c %Z") << "(local: " << std::put_time(std::localtime(&matching_timestamps[i]), "%c %Z") << ")" << std::endl;

                std::cout << "Please specify if you want to use one of the stored results for the metric evaluation: " << std::endl;
                std::cout << "Allowed input: comma-separated numbers (e.g., \"0\", \"0,1\") or \"*\" to evaluate all stored results" << std::endl; 
                std::cout << "               use \"n\" or \"N\" to generate new results" << std::endl;
                std::cout << "Input: "; 
                std::string choice; 
                std::cin >> choice; 

                std::vector<size_t> chosen; 
                if ( choice == "*" ) {
                    for ( size_t i = 0; i < matching_timestamps.size(); ++i )
                        chosen.push_back(i);
                } else if ( choice == "n" || choice == "N" ) {
                    return true;
                } else {
                    std::stringstream ss(choice);
                    size_t x; 
                    while ( ss >> x ) {
                        assert(x < matching_timestamps.size() && "Invalid choice!"); 
                        chosen.push_back(x);
                        if ( ss.peek() == ',' ) {
                            ss.ignore();
                        }
                    }
                }
                for (const auto& i : chosen) {
                    timestamps_.push_back(matching_timestamps[i]); //store time stamps to evaluate
                }
            } 
            else {
                std::cout << "No stored files for workflow " << workflow_identifier_ << " were found!" << std::endl;
                return true;
            }
            return false;
        }


    }
}