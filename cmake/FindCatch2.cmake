include(FetchContent)

# 1. Declare the fmt library
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG        "69e0473"  # Use a specific version tag for stability
)

# 2. Download and add to the project
FetchContent_MakeAvailable(Catch2)
