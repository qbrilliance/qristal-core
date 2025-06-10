set(source_files
  src/noise_model/noise_channel.cpp
  src/noise_model/noise_model.cpp
  src/noise_model/default_noise_model.cpp
)

set(headers
  include/qristal/core/noise_model/json_complex_convert.hpp
  include/qristal/core/noise_model/noise_channel.hpp
  include/qristal/core/noise_model/noise_model.hpp
  include/qristal/core/noise_model/noise_properties.hpp
  include/qristal/core/noise_model/readout_error.hpp
)

set(LIBRARY_NAME noise)
add_library(${LIBRARY_NAME} SHARED ${source_files} ${headers})
add_library(${NAMESPACE}::${PROJECT_NAME}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})
set_target_properties(${LIBRARY_NAME}
PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK TRUE
    INSTALL_RPATH "$ORIGIN;${CMAKE_INSTALL_RPATH};${XACC_ROOT}/lib"
)

# Include dependencies.
target_include_directories(${LIBRARY_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    nlohmann::json
    Eigen3::Eigen
    range-v3::range-v3
)

# Install the library
install(
  TARGETS ${LIBRARY_NAME}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${qristal_core_LIBDIR}
  EXPORT ${LIBRARY_NAME}Targets
)

# Install the Targets.cmake file for the library
set(noiseTargetsFile "${LIBRARY_NAME}Targets.cmake")
install(
  EXPORT ${LIBRARY_NAME}Targets
  FILE ${noiseTargetsFile}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
  NAMESPACE ${NAMESPACE}::${PROJECT_NAME}::
)
