include(FetchContent)

FetchContent_Declare(
    Hydrolib
    GIT_REPOSITORY https://github.com/SeaJackal/Hydrolib-soft.git
    GIT_TAG 1c21669707ba36e2cd95d37db00bafc62fdc1d51
)

FetchContent_MakeAvailable(Hydrolib)
