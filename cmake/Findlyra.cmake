include(FetchContent)

# 1. Declare the fmt library
FetchContent_Declare(
  lyra
  GIT_REPOSITORY https://github.com/bfgroup/Lyra
  GIT_TAG        "1.7.0"  # Use a specific version tag for stability
)

# 2. Download and add to the project
FetchContent_MakeAvailable(lyra)
