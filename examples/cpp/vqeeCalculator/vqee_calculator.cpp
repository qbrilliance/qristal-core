// Copyright (c) Quantum Brilliance Pty Ltd
#include "qb/core/optimization/vqee/vqee.hpp"
#include <args.hxx>

#include <iomanip>
#include <iostream>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

using qb::vqee::operator<<; // cout overload for vector<>

namespace jsonExamples {
    json H2example1 = json::parse(R"({
        "pauli": "-1.04235464570829 + 0.18125791479311 X0 + -0.78864539363997 Z0",
        "circuit": ".compiler xasm\n.circuit ansatz\n.parameters theta\n.qbit q\nRy(q[0], theta);",
        "nQubits": 1,
        "theta": 0.1
    })");

    json H2example2 = json::parse(R"({
        "geometry": "H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486",
        "ansatz": "UCCSD",
        "nQubits": 4,
        "nElectrons": 2,
        "thetas": [0.1, 0.2, 0.3]
    })");

    // should throw: geometry xor pauli
    json H2throw1 = json::parse(R"({
        "geometry": "H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486",
        "pauli": "-1.04235464570829 + 0.18125791479311 X0 + -0.78864539363997 Z0",
        "ansatz": "UCCSD",
        "nQubits": 4,
        "nElectrons": 2
    })");

    // should throw: ansatz xor circuit
    json H2throw2 = json::parse(R"({
        "pauli": "-1.04235464570829 + 0.18125791479311 X0 + -0.78864539363997 Z0",
        "ansatz": "UCCSD",
        "circuit": ".compiler xasm\n.circuit ansatz\n.parameters theta\n.qbit q\nRy(q[0], theta);",
        "nQubits": 4,
        "nElectrons": 2
    })");
}


std::vector<double> parseVector(const std::size_t nOptParams, const std::string& str, const bool isDebug) {
    if ( isDebug ) { std::cout << "parsing: " << nOptParams << " elements" << std::endl; }
    std::vector<double> vec{};
    vec.reserve(nOptParams);
    const std::size_t strLen = str.size();
    std::string::size_type xPos = 0; // start of substring
    std::string::size_type nPos = str.size();  // end of substring
    const std::string delimiter = ",";
    // go from left to right, find delimiter ";" and parse substring to double into the vector element
    for (std::size_t i=0; i<nOptParams; i++) {
        nPos = str.find(delimiter, xPos);
        if (nPos != std::string::npos) { // substring found: go on
            std::string substr = str.substr(xPos,nPos-xPos);
            double element = std::stod(substr);
            if ( isDebug ) { std::cout << i << ": \"" << substr <<"\": " << std::stod(substr) << std::endl; }
            vec.emplace_back(element);
            xPos = nPos+1;
        } else { // special case: string does not end on delimiter or has less elements than nOptParams
            if ( isDebug ) { std::cout << "end of string reached" << std::endl; }
            // fail if this is not the last element or the remaining string is empty
            if ( i < nOptParams-1 || xPos >= strLen ) { 
                throw std::invalid_argument("\nProvided initial vector is too short or using wrong delimiter (must be \",\")\n"); 
            }
            std::string substr = str.substr(xPos); // try to get last element
            double element = std::stod(substr); 
            if ( isDebug ) { std::cout << i << ": \"" << substr <<"\": " << std::stod(substr) << std::endl; }
            vec.emplace_back(element);
        }
    }
    // special case: string has more elements than requested
    if ( xPos == nPos+1 && nPos < strLen-1 ) { throw std::invalid_argument("\nProvided initial vector is too long\n"); }

    return vec;
}

template <typename T>
void readOutIfAvailable(T& out, const json& o, const std::string& key, const bool isRoot) {
    if (o.count(key)) {
        out = o[key].get<T>();
        if (isRoot) { std::cout << "setting " << key << " = " << out << std::endl; }
    }
    else {
        if (isRoot) { std::cout << "using default " << key << " = " << out << std::endl; }
    }
}

template <typename T1, typename T2>
void setOptionIfAvailable(T1& val, T2& CLIkey, const std::string& name, const bool isRoot) {
    if (CLIkey) { 
        val = args::get(CLIkey);
        if (isRoot) { std::cout << "\nsetting " << name << " = " << val << std::endl; }
    }
}

std::string deescapeString(const std::string& str) {
    std::stringstream ss;
    for (auto it = str.begin(); it != str.end(); it++) {
        if ( *it == '\\') { 
            ss << '\n';
            it++;
        } else {
            ss << *it;
        }
    }
    return ss.str();
}

std::string escapeString(const std::string& str) {
    std::stringstream ss;
    for (auto it = str.begin(); it != str.end(); it++) {
        if ( *it == '\n') { 
            ss << "\\n";
        } else {
            ss << *it;
        }
    }
    return ss.str();
}


// solution: -1.137
// possible input variations:
// get problem from json file, no other settings from command line interface
    // ./vqeeCalculator --fromJson=/mnt/qb/core/examples/cpp/vqeeCalculator/tests.json --jsonID=0

// with default ansatz, circuit and pauli: allows for --nThreads and --theta or --thetas
    // ./vqeeCalculator
    // ./vqeeCalculator --nThreads=2 --thetas="0.1, 0.2, 0.1"
    // ./vqeeCalculator --nThreads=2 --theta=0.2

// setting ansatz, circuit and pauli
    // ./vqeeCalculator --nThreads=2 --theta=0.1 --nQubits=4 --geometry="H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486" --ansatz="UCCSD" --nElectrons=2
    // ./vqeeCalculator --maxIters=500 --nThreads=2 --theta=0.25 --nQubits=4 --geometry="H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486" --circuit=".compiler xasm\n.circuit ansatz\n.parameters P\n.qbit q\nX(q[0]);\nX(q[1]);\nRz(q[0], P[2]);\nRx(q[0], 0.5*pi);\nRz(q[0], P[0]);\nRx(q[0], -0.5*pi);\nRz(q[0], P[1]);\nRz(q[1], P[5]);\nRx(q[1], 0.5*pi);\nRz(q[1], P[3]);\nRx(q[1], -0.5*pi);\nRz(q[1], P[4]);\nRz(q[2], P[8]);\nRx(q[2], 0.5*pi);\nRz(q[2], P[6]);\nRx(q[2], -0.5*pi);\nRz(q[2], P[7]);\nRz(q[3], P[11]);\nRx(q[3], 0.5*pi);\nRz(q[3], P[9]);\nRx(q[3], -0.5*pi);\nRz(q[3], P[10]);\nCNOT(q[0], q[1]);\nCNOT(q[1], q[2]);\nCNOT(q[2], q[3]);\nRz(q[0], P[14]);\nRx(q[0], 0.5*pi);\nRz(q[0], P[12]);\nRx(q[0], -0.5*pi);\nRz(q[0], P[13]);\nRz(q[1], P[17]);\nRx(q[1], 0.5*pi);\nRz(q[1], P[15]);\nRx(q[1], -0.5*pi);\nRz(q[1], P[16]);\nRz(q[2], P[20]);\nRx(q[2], 0.5*pi);\nRz(q[2], P[18]);\nRx(q[2], -0.5*pi);\nRz(q[2], P[19]);\nRz(q[3], P[23]);\nRx(q[3], 0.5*pi);\nRz(q[3], P[21]);\nRx(q[3], -0.5*pi);\nRz(q[3], P[22]);\nCNOT(q[0], q[1]);\nCNOT(q[1], q[2]);\nCNOT(q[2], q[3]);\n"

    // change Pauli (atm solution = -1.851) and increase nIters for UCCSD ansatz
        // ./vqeeCalculator --maxIters=100 --nThreads=2 --theta=0.1 --nQubits=4 --pauli="5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1" --ansatz="UCCSD" --nElectrons=2
        // ./vqeeCalculator --nThreads=2 --theta=0.1 --nQubits=1 --pauli="-1.04235464570829 + 0.18125791479311 X0 + -0.78864539363997 Z0" --circuit=".compiler xasm\n.circuit ansatz\n.parameters theta\n.qbit q\nRy(q[0], theta);" 

// should fail: 
// thetas too short:            ./vqeeCalculator --nThreads=2 --thetas="0.1, 0.2"
// thetas too long:             ./vqeeCalculator --nThreads=2 --thetas="0.1, 0.2, 0.1, 0.2"
// theta and thetas provided:   ./vqeeCalculator --nThreads=2 --theta=0.2 --thetas="0.1, 0.2, 0.1"
// pauli and geometry given:    ./vqeeCalculator --nThreads=2 --geometry="H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486" --pauli="5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1" --ansatz="UCCSD" --theta=0.2 --nQubits=4 --nElectrons=2
// circuit and ansatz given:    ./vqeeCalculator --nThreads=2 --pauli="5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1" --circuit=".compiler xasm\n.circuit ansatz\n.parameters theta\n.qbit q\nRy(q[0], theta);" --ansatz="UCCSD" --theta=0.2 --nQubits=4 --nElectrons=2
int main (int argc, char *argv[]) {
    xacc::Initialize(); //xacc::Initialize(argc, argv);
    xacc::external::load_external_language_plugins();
    xacc::set_verbose(false);

    const bool isRoot = GetRank() == 0;
    if (isRoot) {
        if (isMPIEnabled()) {
            printf("MPI_enabled\n");
        }
        else {
            printf("not MPI_enabled\n");
        }
    }
    
    // std::string HEA_string = qb::vqee::HEA_String(4, 2);
    // std::cout << "\nHEA(4,2): \n" << escapeString(HEA_string) << '\n'<< std::endl;
    
// - - - - - - - - - - - - - - - parse options - - - - - - - - - - - - - - - //
    args::ArgumentParser parser("VQEE-Calculator: Calculates the solution of user specified molecule chain with Variational Quantum Eigenvalue solver.", "See source code comments above main for CLI examples.");

    // parallelization
    args::ValueFlag<int>          CLInThreads(parser, "nThreads", "Number of threads per MPI process, e.g. 1 or 2", {"nThreads"}); 

    // Pauli
    args::ValueFlag<std::string>  CLIgeometry(parser, "geometry", "Molecule geometry in Angstrom, e.g.: \"H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486\"", {"geometry"});
    args::ValueFlag<std::string>     CLIpauli(parser, "pauli", "Custom Pauli string, e.g. for H2: \"5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1\"", {"pauli"});

    // circuit
    args::ValueFlag<std::string>    CLIansatz(parser, "ansatzType", "Ansatz type: must be \"UCCSD\", \"ASWAP\" or \"HEA\"", {"ansatz"});    
    args::ValueFlag<int>           CLInQubits(parser, "nQubits", "Number of qubits in ansatz, e.g. 1 or 2", {"nQubits"});
    args::ValueFlag<int>        CLInElectrons(parser, "nElectrons", "Number of electrons in ansatz, e.g. 1 or 2", {"nElectrons"});
    args::ValueFlag<std::string>   CLIcircuit(parser, "circuit", "Custom circuit string", {"circuit"});
    
    // initial parameters
    args::ValueFlag<std::string>    CLIthetas(parser, "thetas", "Initial parameters vector (double), e.g. \"0.1, 0.2, -0.3, 1, 2\"", {"thetas"});
    args::ValueFlag<double>          CLItheta(parser, "theta", "Initial parameter uniform value (double), e.g. 0.1", {"theta"});  
    
    // VQE options
    args::ValueFlag<int>            CLInShots(parser, "nShots", "Number of shots (set to 1 for deterministic run), e.g. 1000", {"nShots"});
    args::ValueFlag<int>          CLImaxIters(parser, "maxIters", "Max optimizer iterations, e.g. 50", {"maxIters"});
    args::ValueFlag<double>      CLItolerance(parser, "tolerance", "Optimizer tolerance, e.g. 1E-6", {"tolerance"});  
    
    args::HelpFlag                       CLIh(parser, "help", "help", {"help"});
    args::Flag                     CLIverbose(parser, "verbose", "", {"verbose"});
    args::ValueFlag<std::string>  CLIfromJson(parser, "fromJson", "Read all options from json file instead: specify [PATH]", {"fromJson"});
    args::ValueFlag<std::string>  CLIoutputJson(parser, "outputJson", "Output results to a json file with path and name given by --outputJson", {"outputJson"});
    args::ValueFlag<std::size_t>    CLIjsonID(parser, "jsonID", "ID [0...N] of json object in array of json objects in provided json file", {"jsonID"});

    try {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Help&) {
        if (isRoot) { std::cout << parser << std::endl; }
        return 0;
    }
    catch (const args::ParseError& e) {
        if (isRoot) { std::cerr << e.what() << '\n' << parser << std::endl; }
        return 1;
    }
    catch (const args::ValidationError& e) {
        if (isRoot) { std::cerr << e.what() << '\n' << parser << std::endl; }
        return 1;
    }

// - - - - - - - - - - - - - set params with read out options - - - - - - - - - - - - - - //
    // default is deterministic, 1 shot, 50 maxIters, 1E-6 tolerance and not partitioned
    qb::vqee::Params params{};

    // the defaults will be overwritten if provided by CLI
    params.nWorker = GetSize(); // get number of MPI processes
    params.nThreadsPerWorker = 1; // set default number of threads to 1
    std::string geometry = qb::vqee::hydrogenChainGeometry(2); 
    params.pauliString = qb::vqee::pauliStringFromGeometry(geometry, "sto-3g");
    qb::vqee::AnsatzID ansatzID = qb::vqee::AnsatzID::UCCSD;
    params.nQubits = 4;
    int nElectrons = 2;
    bool isVerbose = false;
    std::size_t nOptParams = 0;

    if (CLIverbose) {
        isVerbose = true;
        if (isRoot) { std::cout << "setting verbose" << std::endl; }
    }

    if (CLIfromJson) {
        const std::string jsonPath = args::get(CLIfromJson);
        if (isRoot) { std::cout << "retrieving all settings from json file: " << jsonPath << std::endl; }

        // load whole json file
        std::ifstream i(jsonPath);
        json jsonList;
        i >> jsonList;

        // assume list of json objects (problem definitions) in json file. Reduces to one element list.
        std::size_t jsonID = 0;
        if ( CLIjsonID ) {
            jsonID = args::get(CLIjsonID);
        } else {
            if (isRoot) { throw std::invalid_argument("\nNo jsonID provided! We assume list of json objects: [{}], or [{},{},...]\n"); }
        }

        json jsonObj;
        // bounds checked access
        try {
            jsonObj = jsonList.at(jsonID);
        }
        catch (json::out_of_range& e) {
            if (isRoot) { std::cout << e.what() << std::endl; }
        }
        // check if object type
        if ( !jsonObj.is_object() ) {
            if (isRoot) { throw std::invalid_argument("\nChosen element in json list is not of object type! We assume a file containing a list of json objects: [{}], or [{},{},...]\n"); }
        }


        // cache which entries are available:  
        const bool contains_nQubits = jsonObj.count("nQubits");
        const bool contains_geometry = jsonObj.count("geometry");
        const bool contains_pauli = jsonObj.count("pauli");
        const bool contains_ansatz = jsonObj.count("ansatz");
        const bool contains_nElectrons = jsonObj.count("nElectrons");
        const bool contains_circuit = jsonObj.count("circuit");
        const bool contains_theta = jsonObj.count("theta");
        const bool contains_thetas = jsonObj.count("thetas");

        // checking for allowed combinations
        if ( !( contains_nQubits 
                && (contains_geometry ^ contains_pauli ) 
                && ( (contains_ansatz && contains_nElectrons) ^ contains_circuit )
            )) {
            if (isRoot) { throw std::invalid_argument("\nNo valid configuration! Valid combinations are: nQubits + geometry or pauli + (ansatz +nElectrons) or circuit!\n"); }            
        }
        
        // setting pauli
        params.nQubits = jsonObj["nQubits"].get<int>();
        if (isRoot) { std::cout << "setting nQubits = " << params.nQubits << std::endl; }

        if (contains_pauli) {
            params.pauliString = jsonObj["pauli"].get<std::string>();
            if (isRoot) { std::cout << "setting Pauli = " << params.pauliString << std::endl; }
        } 
        else if (contains_geometry) {
            geometry = jsonObj["geometry"].get<std::string>();
            if (isRoot) { std::cout << "setting geometry = " << geometry << std::endl; }
            
            params.pauliString = qb::vqee::pauliStringFromGeometry(geometry, "sto-3g");
            if (isRoot) { std::cout << "setting Pauli = "<< params.pauliString << std::endl; }
        } 
        else {
            if (isRoot) { throw std::invalid_argument("\nNo valid configuration!\n"); } // should not happen
        }
            
        // setting circuit/ansatz
        if ( contains_circuit ) {
            params.circuitString = deescapeString(jsonObj["circuit"].get<std::string>());
            xacc::qasm(params.circuitString); 
            params.ansatz = xacc::getCompiled("ansatz");
            if (isRoot) { std::cout << "\ncompiled ansatz = "<< params.ansatz->toString() << std::endl; }
            nOptParams = params.ansatz->getVariables().size();
        }
        else if ( contains_ansatz && contains_nQubits && contains_nElectrons ){
            ansatzID = qb::vqee::getEnumFromName(jsonObj["ansatz"]);
            nElectrons = jsonObj["nElectrons"].get<int>();
            if (isRoot) { 
                std::cout << "\nsetting ansatz = " << qb::vqee::getEnumName(ansatzID) 
                        << "with nQubits = " << params.nQubits 
                        << " and nElectrons = " << nElectrons << std::endl; 
            }
            nOptParams = qb::vqee::setAnsatz(params, ansatzID, params.nQubits, nElectrons);
            if (isRoot) { std::cout << "\nsetting circuitString = "<< params.circuitString << std::endl; }
        }
        else {
            if (isRoot) { throw std::invalid_argument("\nNo valid configuration!\n"); } // should not happen
        }

        // setting initial values
        if (contains_thetas && contains_theta) { 
            if (isRoot) { throw std::invalid_argument("\nOnly one option of --theta and --thetas is allowed!\n");  }
        }
        if (contains_thetas) { 
            params.theta = jsonObj["thetas"].get<std::vector<double>>();
            if (isRoot) { std::cout << "\nsetting thetas = " << params.theta << std::endl; }
        }
        else {
            if (contains_theta) {
                const double theta = jsonObj["theta"].get<double>();
                if (isRoot) { std::cout << "\nsetting uniform theta = " << theta << std::endl; }
                params.theta = std::vector<double>(nOptParams, theta);
            } else {
                params.theta = std::vector<double>(nOptParams, 0.1); // set default uniform thetas 
                if (isRoot) { std::cout << "\nusing default uniform initial parameters (value = 0.1)" << std::endl; }
            }
        }

        // optional settings:
        readOutIfAvailable(params.nThreadsPerWorker, jsonObj, "nThreads", isRoot);
        readOutIfAvailable(params.maxIters, jsonObj, "maxIters", isRoot);
        readOutIfAvailable(params.tolerance, jsonObj, "tolerance", isRoot);
        readOutIfAvailable(params.nShots, jsonObj, "nShots", isRoot);
        params.isDeterministic = params.nShots<=1;
        readOutIfAvailable(params.partitioned, jsonObj, "partitioned", isRoot);
    } 
    else {
        // setting pauli, ansatz and circuit
        // no options except initial parameters given: using H2 default case
        if (  !( CLInQubits || CLIgeometry || CLIpauli || CLIansatz || CLIcircuit || CLInElectrons ) ) { 
            if (isRoot) { std::cout << "using default values: H_2 molecule with 1.4 Bohr distance, UCCSD ansatz, 4 qubits and 2 electrons " << std::endl; }
            nOptParams = qb::vqee::setAnsatz(params, ansatzID, params.nQubits, nElectrons); 
        }
        else { // checking for allowed combinations
            if ( !( CLInQubits 
                    && (CLIgeometry ^ CLIpauli ) 
                    && ( (CLIansatz && CLInElectrons) ^ CLIcircuit )
                )) {
                if (isRoot) { throw std::invalid_argument("\nNo valid configuration! Valid combinations are: nQubits + geometry or pauli + (ansatz +nElectrons) or circuit!\n"); }            
            }

            params.nQubits = args::get(CLInQubits);

            if (CLIpauli) {
                params.pauliString = args::get(CLIpauli);
                if (isRoot) { std::cout << "setting Pauli = "<< params.pauliString << std::endl; }
            } 
            else if (CLIgeometry) {
                geometry = args::get(CLIgeometry);
                if (isRoot) { std::cout << "setting geometry = " << geometry << std::endl; }
                
                params.pauliString = qb::vqee::pauliStringFromGeometry(geometry, "sto-3g");
                if (isRoot) { std::cout << "setting Pauli = "<< params.pauliString << std::endl; }
            } 
            else {
                if (isRoot) { throw std::invalid_argument("\nNo valid configuration!\n"); } // should not happen
            }
            
            if ( CLIcircuit ) {
                params.circuitString = deescapeString(args::get(CLIcircuit));
                // if (isRoot) { std::cout << "\nsetting circuitString = \n" << params.circuitString << std::endl; }
                xacc::qasm(params.circuitString); 
                // if (isRoot) { std::cout << "\nsetting qasm " << std::endl; }
                params.ansatz = xacc::getCompiled("ansatz");
                if (isRoot) { std::cout << "\ncompiled ansatz = "<< params.ansatz->toString() << std::endl; }
                nOptParams = params.ansatz->getVariables().size();
            }
            else if ( CLIansatz && CLInQubits && CLInElectrons ){
                ansatzID = qb::vqee::getEnumFromName(args::get(CLIansatz));
                nElectrons = args::get(CLInElectrons);
                if (isRoot) { 
                    std::cout << "\nsetting ansatz = " << qb::vqee::getEnumName(ansatzID) 
                            << "with nQubits = " << params.nQubits 
                            << " and nElectrons = " << nElectrons << std::endl; 
                }
                nOptParams = qb::vqee::setAnsatz(params, ansatzID, params.nQubits, nElectrons);
                if (isRoot) { std::cout << "\nsetting circuitString = "<< params.circuitString << std::endl; }
            }
            else {
                if (isRoot) { throw std::invalid_argument("\nNo valid configuration!\n"); } // should not happen
            }
        }

        // setting initial values
        if (CLIthetas && CLItheta) { 
            if (isRoot) { throw std::invalid_argument("\nOnly one option of --theta and --thetas is allowed!\n");  }
        }
        if (CLIthetas) { 
            params.theta = parseVector(nOptParams, args::get(CLIthetas), isVerbose && isRoot );
            if (isRoot) { std::cout << "\nsetting thetas = " << params.theta << std::endl; }
        }
        else {
            if (CLItheta) {
                const double theta = args::get(CLItheta);
                if (isRoot) { std::cout << "\nsetting uniform theta = " << theta << std::endl; }
                params.theta = std::vector<double>(nOptParams, theta);
            } else {
                params.theta = std::vector<double>(nOptParams, 0.1); // set default uniform thetas 
                if (isRoot) { std::cout << "\nusing default uniform initial parameters (value = 0.1)" << std::endl; }
            }
        }

        // set optional options: read from CLI if available set params.* and print setting statement
        // default is deterministic, 1 shot, 50 maxIters, 1E-6 tolerance and not partitioned
        setOptionIfAvailable(params.nThreadsPerWorker, CLInThreads, "nThreadsPerWorker", isRoot);
        setOptionIfAvailable(params.nShots, CLInShots, "nShots", isRoot);
        setOptionIfAvailable(params.maxIters, CLImaxIters, "maxIters", isRoot);
        setOptionIfAvailable(params.tolerance, CLItolerance, "tolerance", isRoot);
        params.isDeterministic = params.nShots<=1;
        // params.partitioned = true; // enable for cases with many Pauli-terms

    }

// - - - - - - - - - - - - - actual VQE calls - - - - - - - - - - - - - - //
    // The number of threads per worker is not properly passed down to all simulators (yet).
    if (isRoot){ printf("\nExecuting VQE on %i workers with %i threads each.\n\n", params.nWorker, params.nThreadsPerWorker); }

    xacc::ScopeTimer timer_for_cpu("Walltime in ms", false);
    
    qb::vqee::VQEE vqe{params};
    vqe.optimize();

    const auto      nIters = params.energies.size();
    const double    cpu_ms = timer_for_cpu.getDurationMs();
    if (isRoot) {
        if (CLIoutputJson) {
            const std::string jsonOutputPath = args::get(CLIoutputJson);
            if (isRoot) {
                std::cout << "Saving results to: " << jsonOutputPath << "\n";
            }
            json outjs;
            std::ofstream outjs_file(jsonOutputPath);
            outjs["theta"] = params.theta;
            outjs["energy"] = params.optimalValue;
            outjs["iterations"] = nIters;
            outjs["walltime_ms"] = cpu_ms;
            outjs["pauli"] = params.pauliString;
            // Add more output fields here....

            outjs_file << std::setw(4) << outjs << "\n";
            outjs_file.flush();
            outjs_file.close();
        }
        else {
            std::cout << "\ntheta: " << params.theta << ", energy: " << params.optimalValue
                      << ", iterations: " << nIters << ", CPU wall-time: " << cpu_ms << " ms" << std::endl;
        }
    }
    xacc::Finalize();
    return 0;
}