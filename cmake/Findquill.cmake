include(FetchContent)

FetchContent_Declare(
  quill
  GIT_REPOSITORY https://github.com/odygrd/quill.git
  GIT_TAG        v3.6.0
)

FetchContent_MakeAvailable(quill)
