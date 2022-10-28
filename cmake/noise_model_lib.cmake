set(source_files
  src/noise_model/noise_channel.cpp
  src/noise_model/noise_model.cpp
  src/noise_model/noise_model_factory.cpp
)

set(headers
  include/qb/core/noise_model/noise_channel.hpp
  include/qb/core/noise_model/noise_model_factory.hpp
  include/qb/core/noise_model/noise_model.hpp
  include/qb/core/noise_model/noise_properties.hpp
)

set(COMPONENT_NAME noise_model)
set(LIBRARY_NAME ${NAMESPACE}${PROJECT_NAME}-${COMPONENT_NAME})
add_library(${LIBRARY_NAME} SHARED ${source_files} ${headers})
add_library(${NAMESPACE}::${PROJECT_NAME}::${COMPONENT_NAME} ALIAS ${LIBRARY_NAME})
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
)

# Link dependencies
target_link_libraries(${LIBRARY_NAME}
PUBLIC
    nlohmann_json::nlohmann_json
    Eigen3::Eigen
)

# Install the library
install(
TARGETS ${LIBRARY_NAME}
DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

# Install the headers
install(
DIRECTORY include
DESTINATION ${CMAKE_INSTALL_PREFIX}
)