include(FetchContent)

FetchContent_Declare(
  quill
  GIT_REPOSITORY https://github.com/odygrd/quill.git
  # GIT_TAG        v11.1.0
  GIT_TAG        c57e2f5
)

FetchContent_MakeAvailable(quill)
