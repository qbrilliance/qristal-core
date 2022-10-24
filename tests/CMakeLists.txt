include(add_gtest)

# Commandline interface test
add_gtest(coreCLI)
target_link_libraries(coreCLITester qb::core)

# Noise Model test
add_gtest(NoiseModel PATH noise_model)
target_link_libraries(NoiseModelTester qb::core)

# Add tests defined in sub-dirs
# TODO: hoist them up to this CMakeLists.txt
add_subdirectory(sparse_simulator)
add_subdirectory(uccsd)
add_subdirectory(vqe)
