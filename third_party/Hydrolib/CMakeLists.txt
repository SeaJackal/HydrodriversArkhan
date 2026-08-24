include(FetchContent)

FetchContent_Declare(
    Hydrolib
    GIT_REPOSITORY https://github.com/SeaJackal/Hydrolib-soft.git
    GIT_TAG 3630c3fe0d7feb593bfe7fa0dd26d3a84c85ee85
)

FetchContent_MakeAvailable(Hydrolib)
