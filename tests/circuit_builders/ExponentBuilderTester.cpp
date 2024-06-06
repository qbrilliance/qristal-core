// Copyright (c) 2022 Quantum Brilliance Pty Ltd
//#include "Circuit.hpp"
#include "qb/core/circuit_builders/exponent.hpp"
//#include "xacc.hpp"
//#include "xacc_service.hpp"
#include <cassert>
#include <cstdlib>
#include <gtest/gtest.h>
#include <iostream>
#include <ostream>
#include <random>
//#include <bitset>

TEST(ExponentCircuitTester_1, checkSimple) {
  // Test Exponent: input a bitstring to compare to BestScore.
  bool log_zero = true;
  for (int j = 0; j < 2; j++) {
    for (int l = 0; l < 2; l++) {
      for (int m = 0; m < 2; m++) {
        if (log_zero) {
          log_zero = !log_zero;
          continue;
        }
        else {
          if ((j*4 + l*2 + m) > 4) { continue; }
          }
        qb::CircuitBuilder test_builder;
        int nb_qubits_log = 1 ;
        int log_value = 0;
        if (l==1){
          test_builder.X(1);
          //std::cout << "X " << 1 << std::endl;
          log_value += 2;
          nb_qubits_log = 2;
        }
        if (j==1){
          test_builder.X(2);
          //std::cout << "X " << 2 << std::endl;
          log_value += 4 ;
          nb_qubits_log = 3;
        }
        if (m==1){
          test_builder.X(0);
          //std::cout << "X " << 0 << std::endl;
          log_value++ ;
        }
        std::cout << "\n*** Testing log_value:" << log_value << " ***" << std::endl;
        std::vector<int> qubits_log ;
        for (int qindex = 0; qindex < nb_qubits_log; qindex++){
          qubits_log.push_back(qindex);
          //std::cout << qindex << qubits_log.size() << "\n";
        }

        const xacc::HeterogeneousMap &map = {{"qubits_log",qubits_log}, {"is_LSB", false}};
        qb::Exponent build;
        const bool expand_ok = build.expand(map);

        test_builder.append(build);
        int nb_qubits_exp = build.nb_qubits_exp;
        int nb_qubits = nb_qubits_exp + nb_qubits_log;
        //Check calculation of exponent qubits
        //std::cout << "nb_qubits_log:" << nb_qubits_log << " nb_qubits_exp:" << nb_qubits_exp << std::endl;

        test_builder.MeasureAll(nb_qubits);

        std::vector<int> qubits ;
        for (int qindex = 0; qindex < nb_qubits; qindex++){
          qubits.push_back(qindex);
        }

        // Simulation test:
        // Construct the full circuit including preparation of input trial score
        auto circuit = test_builder.get();  // gateRegistry->createComposite("sim_comp");

        // Add comp:
        std::cout << "HOWDY: Comparator circuit:\n";
        std::cout << "Testing log_value:" << log_value << std::endl;
        //std::cout << circuit->toString() << '\n';
        auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
        auto buffer = xacc::qalloc(nb_qubits);
        acc->execute(buffer, circuit);
        buffer->print();
        EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
        std::string expected_measurement;
        int exp_value = pow(2,log_value);
        std::cout << "log_value:" << log_value << " exp_value:" << exp_value << std::endl;
        int qindex;
        for (int i = 0; i < nb_qubits_exp; i++) {
          qindex = qubits[i];
          if (exp_value&((int) pow(2,qindex))) {
            expected_measurement.push_back('1');
          }
          else {
            expected_measurement.push_back('0');
          }
        }
        for (int i = 0; i < nb_qubits_log; i++){
          if (log_value&((int) pow(2,i))) {
            expected_measurement.push_back('1');
          }
          else {
            expected_measurement.push_back('0');
          }
        }
        std::cout << "expected_measurement:" << expected_measurement << std::endl;
        assert(buffer->getMeasurementCounts()[expected_measurement] == 1024);
        std::cout << "Successfully found two to the power of " << log_value << std::endl;
      }
    }
  }
}

TEST(ExponentCircuitTester_2, checkSimple) {
  // Test Exponent: input a bitstring to compare to BestScore.
  // If input > BestScore, flag qubit should return |1>.
  // Otherwise flag qubit should return |0>.
  bool log_zero = true;
  std::random_device rand_dev ;
  std::uniform_int_distribution<int> distr(2,4) ;
  for (int j = 0; j < 2; j++) {
    for (int l = 0; l < 2; l++) {
      for (int m = 0; m < 2; m++) {
        if (log_zero) {
          log_zero = !log_zero;
          continue;
        }
        else {
          if ((j*4 + l*2 + m) > 4) { continue; }
        }
        qb::CircuitBuilder test_builder;
        int nb_qubits_log = 1 ;
        int log_value = 0;
        if (l==1){
          test_builder.X(1);
          //std::cout << "X " << 1 << std::endl;
          log_value += 2;
          nb_qubits_log = 2;
        }
        if (j==1){
          test_builder.X(2);
          //std::cout << "X " << 2 << std::endl;
          log_value += 4 ;
          nb_qubits_log = 3;
        }
        if (m==1){
          test_builder.X(0);
            //std::cout << "X " << 0 << std::endl;
            log_value++ ;
        }
        std::cout << "\n*** Testing log_value:" << log_value << " ***" << std::endl;
        std::vector<int> qubits_log ;
        for (int qindex = 0; qindex < nb_qubits_log; qindex++){
          qubits_log.push_back(qindex);
        }
        int min_significance_ = distr(rand_dev);

        std::cout << "log.size():" << qubits_log.size() << std::endl;
        const xacc::HeterogeneousMap &map = {{"qubits_log",qubits_log}, {"min_significance", min_significance_}, {"is_LSB", false}};
        qb::Exponent build;
        const bool expand_ok = build.expand(map);
        if (!expand_ok) {
          std::cout << "**** log value:" << log_value << " exp value:" << pow(2, log_value) << " ****" << std::endl;
          assert(nb_qubits_log < min_significance_);
          continue;
        }

        test_builder.append(build);
        int nb_qubits_exp = max(build.nb_qubits_exp + min_significance_ - 1, min_significance_ - 1);
        int nb_qubits = build.nb_qubits_exp + nb_qubits_log;
        //Check calculation of exponent qubits
        //std::cout << "nb_qubits_log:" << nb_qubits_log << " nb_qubits_exp:" << nb_qubits_exp << std::endl;

        test_builder.MeasureAll(nb_qubits);

        std::vector<int> qubits ;
        for (int qindex = 0; qindex < nb_qubits - min_significance_ + 1; qindex++){
          qubits.push_back(qindex);
        }

        // Simulation test:
        // Construct the full circuit including preparation of input trial score
        auto circuit = test_builder.get();  // gateRegistry->createComposite("sim_comp");

        // Add comp:
        std::cout << "HOWDY: Comparator circuit:\n";
        std::cout << "Testing log_value:" << log_value << std::endl;
        auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
        auto buffer = xacc::qalloc(nb_qubits);
        acc->execute(buffer, circuit);
        buffer->print();
        EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
        std::string expected_measurement;
        int exp_value = pow(2,log_value);
        std::cout << "log_value:" << log_value << " exp_value:" << exp_value << std::endl;
        int qindex;
        for (int i = min_significance_-1; i < nb_qubits_exp ; i++) {
          qindex = qubits[i];
          if (exp_value&((int) pow(2,qindex))) {
            expected_measurement.push_back('1');
          }
          else {
            expected_measurement.push_back('0');
          }
        }
        for (int i = 0; i < nb_qubits_log; i++){
          if (log_value&((int) pow(2,i))) {
            expected_measurement.push_back('1');
          }
          else {
            expected_measurement.push_back('0');
          }
        }
        std::cout << "expected_measurement:" << expected_measurement << std::endl;
        assert(buffer->getMeasurementCounts()[expected_measurement] == 1024);
        std::cout << "Successfully found two to the power of " << log_value << std::endl;
      }
    }
  }
}

TEST(ExponentCircuitTester_3, checkSimple) {
  // Test Exponent: input a bitstring to compare to BestScore.
  bool log_zero = true;
  for (int j = 0; j < 2; j++) {
    for (int l = 0; l < 2; l++) {
      for (int m = 0; m < 2; m++) {
        if (log_zero) {
          log_zero = !log_zero;
          continue;
        }
        else {
          if ((j*4 + l*2 + m) > 4) { continue; }
        }
        std::cout << "j:" << j << " l:" << l << " m:" << m << std::endl;
        qb::CircuitBuilder test_builder;
        int nb_qubits_log = 1 ;
        int log_value = 0;
        if (j==1){
          log_value += 4 ;
          nb_qubits_log = 3;
        }
        if (l==1){
          log_value += 2;
          nb_qubits_log = max(2, nb_qubits_log);
        }
        if (m==1){
          log_value++ ;
          nb_qubits_log = max(1, nb_qubits_log);
        }
        int nb_qubits_init = pow(2, pow(2,nb_qubits_log-1));
        if (j==1){
          test_builder.X(nb_qubits_init - 3);
          //std::cout << "jX " << nb_qubits_init - 3 << std::endl;
        }
        if (l==1){
          test_builder.X(nb_qubits_init - 2);
          //std::cout << "lX " << nb_qubits_init - 2 << std::endl;
        }
        if (m==1){
          test_builder.X(nb_qubits_init - 1);
          //std::cout << "mX " << nb_qubits_init - nb_qubits_log << std::endl;
        }

        std::cout << "\n*** Testing log_value:" << log_value << " ***" << std::endl;
        std::vector<int> qubits_log ;
        for (int qindex = nb_qubits_init; qindex > (nb_qubits_init - nb_qubits_log); qindex--){
          qubits_log.push_back(qindex-1);
          //std::cout << qindex-1 << " " << qubits_log.size() << "\n";
        }

        const xacc::HeterogeneousMap &map = {{"qubits_log",qubits_log}, {"is_LSB", true}};
        qb::Exponent build;
        const bool expand_ok = build.expand(map);

        test_builder.append(build);
        int nb_qubits_exp = build.nb_qubits_exp;
        int nb_qubits = nb_qubits_exp + nb_qubits_log;
        //Check calculation of exponent qubits
        std::cout << "nb_qubits_log:" << nb_qubits_log << " nb_qubits_exp:" << nb_qubits_exp << std::endl;

        test_builder.MeasureAll(nb_qubits);

        std::vector<int> qubits ;
        for (int qindex = 0; qindex < nb_qubits; qindex++){
            qubits.push_back(qindex);
        }

        // Simulation test:
        // Construct the full circuit including preparation of input trial score
        auto circuit = test_builder.get();  // gateRegistry->createComposite("sim_comp");

        // Add comp:
        std::cout << "HOWDY: Comparator circuit:\n";
        std::cout << "Testing log_value:" << log_value << std::endl;
        //std::cout << circuit->toString() << '\n';
        auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
        auto buffer = xacc::qalloc(nb_qubits);
        acc->execute(buffer, circuit);
        buffer->print();
        EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
        std::string expected_measurement;
        int exp_value = pow(2,log_value);
        std::cout << "log_value:" << log_value << " exp_value:" << exp_value << std::endl;
        int qindex;
        for (int i = nb_qubits_exp - 1; i >= 0; i--) {
          qindex = qubits[i];
          if (exp_value&((int) pow(2,qindex))) {
            expected_measurement.push_back('1');
          }
          else {
            expected_measurement.push_back('0');
          }
        }
        for (int i = nb_qubits_log - 1; i >= 0; i--){
          if (log_value&((int) pow(2,i))) {
            expected_measurement.push_back('1');
          }
          else {
            expected_measurement.push_back('0');
          }
        }
        std::cout << "expected_measurement:" << expected_measurement << std::endl;
        assert(buffer->getMeasurementCounts()[expected_measurement] == 1024);
        std::cout << "Successfully found two to the power of " << log_value << std::endl;
      }
    }
  }
}

TEST(ExponentCircuitTester_4, checkSimple) {
  // Test Exponent: input a bitstring to compare to BestScore.
  bool log_zero = true;
  std::random_device rand_dev ;
  std::uniform_int_distribution<int> distr(2,4) ;
  for (int j = 0; j < 2; j++) {
    for (int l = 0; l < 2; l++) {
      for (int m = 0; m < 2; m++) {
        if (log_zero) {
          log_zero = !log_zero;
          continue;
        }
        else {
          if ((j*4 + l*2 + m) > 4) { continue; }
        }
        //std::cout << "j:" << j << " l:" << l << " m:" << m << std::endl;
        qb::CircuitBuilder test_builder;
        int min_significance_ = distr(rand_dev); //randint(4) ; //3 ;
        int nb_qubits_log = 1 ;
        int log_value = 0;
        if (j==1){
          log_value += 4 ;
          nb_qubits_log = 3;
        }
        if (l==1){
          log_value += 2;
          nb_qubits_log = max(2, nb_qubits_log);
        }
        if (m==1){
          log_value++ ;
        }
        int nb_qubits_init = pow(2, pow(2,nb_qubits_log-1));
        if (j==1){
          test_builder.X(nb_qubits_init - 3);
          //std::cout << "jX " << nb_qubits_init - 3 << std::endl;
        }
        if (l==1){
          test_builder.X(nb_qubits_init - 2);
          //std::cout << "lX " << nb_qubits_init - 2 << std::endl;
        }
        if (m==1){
          test_builder.X(nb_qubits_init - 1);
          //std::cout << "mX " << nb_qubits_init - 1 << std::endl;
        }

        std::cout << "\n*** Testing log_value:" << log_value << " ***" << std::endl;
        std::vector<int> qubits_log ;
        for (int qindex = nb_qubits_init; qindex > (nb_qubits_init - nb_qubits_log); qindex--){
          qubits_log.push_back(qindex-1);
          std::cout << qindex-1 << " " << qubits_log.size() << "\n";
        }

        const xacc::HeterogeneousMap &map = {{"qubits_log",qubits_log}, {"is_LSB", true}, {"min_significance", min_significance_}};
        qb::Exponent build;
        const bool expand_ok = build.expand(map);
        if (!expand_ok) {
          std::cout << "**** log value:" << log_value << " exp value:" << pow(2, log_value) << " ****" << std::endl;
          assert(nb_qubits_log < min_significance_);
          continue;
        }

        test_builder.append(build);
        int nb_qubits_exp = build.nb_qubits_exp + min_significance_ - 1;
        int nb_qubits = nb_qubits_exp + nb_qubits_log;
        //Check calculation of exponent qubits
        //std::cout << "nb_qubits_log:" << nb_qubits_log << " nb_qubits_exp:" << nb_qubits_exp << std::endl;

        test_builder.MeasureAll(nb_qubits);

        std::vector<int> qubits ;
        for (int qindex = 0; qindex < nb_qubits; qindex++){
          qubits.push_back(qindex);
        }

            // Simulation test:
            // Construct the full circuit including preparation of input trial score
        auto circuit = test_builder.get();  // gateRegistry->createComposite("sim_comp");

            // Add comp:
        std::cout << "HOWDY: Comparator circuit:\n";
        std::cout << "Testing log_value:" << log_value << std::endl;
        //std::cout << circuit->toString() << '\n';
        // Sim:
        auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
        auto buffer = xacc::qalloc(nb_qubits);
        acc->execute(buffer, circuit);
        buffer->print();
        EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
        std::string expected_measurement;
        int exp_value = pow(2,log_value);
        std::cout << "log_value:" << log_value << " exp_value:" << exp_value << std::endl;
        int qindex;
        for (int i = 0; i < min_significance_ - 1; i++) {
          expected_measurement.push_back('0');
        }
        for (int i = nb_qubits_exp - 1 - min_significance_ + 1; i >= 0; i--) {
          qindex = qubits[i] + min_significance_ - 1;
          //std::cout << "i:" << i << " qindex:" << qindex << std::endl;
          if (exp_value&((int) pow(2,qindex))) {
            expected_measurement.push_back('1');
            }
          else {
            expected_measurement.push_back('0');
          }
        }
        for (int i = nb_qubits_log - 1; i >= 0; i--){
          if (log_value&((int) pow(2,i))) {
            expected_measurement.push_back('1');
          }
          else {
            expected_measurement.push_back('0');
          }
        }
        std::cout << "expected_measurement:" << expected_measurement << std::endl;
        assert(buffer->getMeasurementCounts()[expected_measurement] == 1024);
        std::cout << "Successfully found two to the power of " << log_value << std::endl;
      }
    }
  }
}

TEST(ExponentCircuitTester_5, checkSimple) {
  // Test Exponent: input a bitstring to compare to BestScore.
  bool log_zero = true;
  std::random_device rand_dev ;
  std::uniform_int_distribution<int> distr(1,4) ;
  for (int j = 0; j < 2; j++) {
    for (int l = 0; l < 2; l++) {
      for (int m = 0; m < 2; m++) {
        if (log_zero) {
          log_zero = !log_zero;
          continue;
        }
        else {
          if ((j*4 + l*2 + m) > 4) { continue; }
        }
        qb::CircuitBuilder test_builder;
        int nb_qubits_log = 1 ;
        int log_value = 0;
        if (l==1){
          test_builder.X(1);
          //std::cout << "X " << 1 << std::endl;
          log_value += 2;
          nb_qubits_log = 2;
        }
        if (j==1){
          test_builder.X(2);
          //std::cout << "X " << 2 << std::endl;
          log_value += 4 ;
          nb_qubits_log = 3;
        }
        if (m==1){
          test_builder.X(0);
          //std::cout << "X " << 0 << std::endl;
          log_value++ ;
        }
        std::cout << "\n*** Testing log_value:" << log_value << " ***" << std::endl;
        int min_significance_ = distr(rand_dev); //randint(4) ; //3 ;
        std::vector<int> qubits_log ;
        std::vector<int> qubits_exponent_ ;
        std::vector<int> qubits_ancilla_ ;
        int nb_qubits_exp = pow(2, pow(2,nb_qubits_log-1));
        if (nb_qubits_exp < (min_significance_ - 1 + nb_qubits_log)) {
          continue;
        }
        for (int qindex = 0; qindex < (nb_qubits_exp - min_significance_ + 1); qindex++){
          qubits_exponent_.push_back(qindex);
          std::cout << qindex << " qubits_exponent:" << qubits_exponent_[qindex] << std::endl;
        }
        int max_exponent_ = *max_element(qubits_exponent_.begin(),qubits_exponent_.end());
        for (int qindex = 0; qindex < nb_qubits_log; qindex++){
          qubits_log.push_back(qindex);
          qubits_ancilla_.push_back(qindex + max_exponent_ + 1);
          std::cout << qindex << qubits_log.size() << " qubits_ancilla:" << qubits_ancilla_[qindex] << std::endl;
        }

        std::cout << "log.size():" << qubits_log.size() << "\n";
        const xacc::HeterogeneousMap &map = {{"qubits_log",qubits_log}, {"qubits_exponent", qubits_exponent_}, {"qubits_ancilla", qubits_ancilla_},
            {"min_significance", min_significance_}, {"is_LSB", false}};
        std::cout << "build\n" ;
        qb::Exponent build;
        const bool expand_ok = build.expand(map);
        if (!expand_ok) {
          //std::cout << "**** log value:" << log_value << " exp value:" << pow(2, log_value) << " ****" << std::endl;
          assert(nb_qubits_log < min_significance_);
          continue;
        }

        test_builder.append(build);
        int nb_qubits = nb_qubits_exp + nb_qubits_log;
        //Check calculation of exponent qubits
        //std::cout << "nb_qubits_log:" << nb_qubits_log << " nb_qubits_exp:" << nb_qubits_exp << std::endl;

        test_builder.MeasureAll(nb_qubits);

        std::vector<int> qubits ;
        for (int qindex = 0; qindex < nb_qubits - min_significance_ + 1; qindex++){
          qubits.push_back(qindex);
        }

        // Simulation test:
        // Construct the full circuit including preparation of input trial score
        auto circuit = test_builder.get();  // gateRegistry->createComposite("sim_comp");

        // Add comp:
        std::cout << "HOWDY: Comparator circuit:\n";
        std::cout << "Testing log_value:" << log_value << std::endl;
        //std::cout << circuit->toString() << '\n';
        // Sim:
        auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
        auto buffer = xacc::qalloc(nb_qubits);
        acc->execute(buffer, circuit);
        buffer->print();
        EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
        std::string expected_measurement;
        int exp_value = pow(2,log_value);
        std::cout << "log_value:" << log_value << " exp_value:" << exp_value << std::endl;
        std::cout << "min_significance_: " << min_significance_ << std::endl;
        int qindex;
        for (int i = min_significance_ - 1; i < nb_qubits_exp; i++) {
          qindex = qubits[i];
          if (exp_value&((int) pow(2,qindex))) {
            expected_measurement.push_back('1');
          }
          else {
            expected_measurement.push_back('0');
          }
        }
        for (int i = 0; i < nb_qubits_log; i++){
          if (log_value&((int) pow(2,i))) {
            expected_measurement.push_back('1');
          }
          else {
            expected_measurement.push_back('0');
          }
        }
        for (int i = 0; i < min_significance_ - 1; i++) expected_measurement.push_back('0');
        std::cout << "expected_measurement:" << expected_measurement << std::endl;
        assert(buffer->getMeasurementCounts()[expected_measurement] == 1024);
        std::cout << "Successfully found two to the power of " << log_value << std::endl;
      }
    }
  }
}

TEST(ExponentCircuitTester_6, checkSimple) {
  // Test Exponent: input a bitstring to compare to BestScore.
  bool log_zero = true;
  std::random_device rand_dev ;
  std::uniform_int_distribution<int> distr(1,4) ;
  for (int j = 0; j < 2; j++) {
    for (int l = 0; l < 2; l++) {
      for (int m = 0; m < 2; m++) {
        if (log_zero) {
          log_zero = !log_zero;
          continue;
        }
        else {
          if ((j*4 + l*2 + m) > 4) { continue; }
        }
        std::cout << "j:" << j << " l:" << l << " m:" << m << std::endl;
        qb::CircuitBuilder test_builder;
        int min_significance_ = 2; //distr(rand_dev); //randint(4) ; //3 ;
        int nb_qubits_log = 1 ;
        int log_value = 0;
        if (j==1){
          log_value += 4 ;
          nb_qubits_log = 3;
        }
        if (l==1){
          log_value += 2;
          nb_qubits_log = max(2, nb_qubits_log);
        }
        if (m==1){
          log_value++ ;
        }
        int nb_qubits_exp = pow(2, pow(2,nb_qubits_log-1));
        if (j==1){
          test_builder.X(nb_qubits_exp - 3);
          //std::cout << "jX " << nb_qubits_exp - 3 << std::endl;
        }
        if (l==1){
          test_builder.X(nb_qubits_exp - 2);
          //std::cout << "lX " << nb_qubits_exp - 2 << std::endl;
        }
        if (m==1){
          test_builder.X(nb_qubits_exp - 1);
          //std::cout << "mX " << nb_qubits_exp - 1 << std::endl;
        }

        std::cout << "\n*** Testing log_value:" << log_value << " ***" << std::endl;
        std::vector<int> qubits_log ;
        std::vector<int> qubits_exponent_ ;
        std::vector<int> qubits_ancilla_ ;
        for (int qindex = nb_qubits_exp; qindex > 0; qindex--){
          qubits_exponent_.push_back(qindex - 1);
        }
        int max_exponent_ = *max_element(qubits_exponent_.begin(),qubits_exponent_.end());
        for (int qindex = nb_qubits_exp; qindex > (nb_qubits_exp - nb_qubits_log); qindex--){
          qubits_log.push_back(qindex-1);
          qubits_ancilla_.push_back(nb_qubits_log + qindex - 1);
          //std::cout << qindex-1 << " " << qubits_log.size() << "qubits_ancilla:" << max_exponent_ + 1 + nb_qubits_log - qindex << std::endl;
        }

        const xacc::HeterogeneousMap &map = {{"qubits_log",qubits_log}, {"qubits_exponent", qubits_exponent_}, {"qubits_ancilla", qubits_ancilla_},
            {"min_significance", min_significance_}, {"is_LSB", true}};
        qb::Exponent build;
        const bool expand_ok = build.expand(map);
        if (!expand_ok) {
          std::cout << "**** log value:" << log_value << " exp value:" << pow(2, log_value) << " ****" << std::endl;
          assert(nb_qubits_log < min_significance_);
          continue;
        }

        test_builder.append(build);
        int nb_qubits = nb_qubits_exp + nb_qubits_log;
        //Check calculation of exponent qubits
        std::cout << "nb_qubits_log:" << nb_qubits_log << " nb_qubits_exp:" << nb_qubits_exp << std::endl;

        test_builder.MeasureAll(nb_qubits);

        std::vector<int> qubits ;
        for (int qindex = 0; qindex < nb_qubits; qindex++){
          qubits.push_back(qindex);
        }

        // Simulation test:
        // Construct the full circuit including preparation of input trial score
        auto circuit = test_builder.get();  // gateRegistry->createComposite("sim_comp");

        // Add comp:
        std::cout << "HOWDY: Comparator circuit:\n";
        std::cout << "Testing log_value:" << log_value << std::endl;
        //std::cout << circuit->toString() << '\n';
        // Sim:
        auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
        auto buffer = xacc::qalloc(nb_qubits);
        acc->execute(buffer, circuit);
        buffer->print();
        EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
        std::string expected_measurement;
        int exp_value = pow(2,log_value);
        std::cout << "log_value:" << log_value << " exp_value:" << exp_value << std::endl;
        int qindex;
        for (int i = 0; i < min_significance_ - 1; i++) {
          expected_measurement.push_back('0');
        }
        for (int i = nb_qubits_exp - 1 - min_significance_ + 1; i >= 0; i--) {
          qindex = qubits[i] + min_significance_ - 1;
          std::cout << "i:" << i << " qindex:" << qindex << std::endl;
          if (exp_value&((int) pow(2,qindex))) {
            expected_measurement.push_back('1');
          }
          else {
            expected_measurement.push_back('0');
          }
        }
        for (int i = nb_qubits_log - 1; i >= 0; i--){
          if (log_value&((int) pow(2,i))) {
            expected_measurement.push_back('1');
          }
          else {
            expected_measurement.push_back('0');
          }
        }
        std::cout << "expected_measurement:" << expected_measurement << std::endl;
        assert(buffer->getMeasurementCounts()[expected_measurement] == 1024);
        std::cout << "Successfully found two to the power of " << log_value << std::endl;
      }
    }
  }
}
