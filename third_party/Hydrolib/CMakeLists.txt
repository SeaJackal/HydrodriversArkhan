include(FetchContent)

FetchContent_Declare(
    Hydrolib
    GIT_REPOSITORY https://github.com/SeaJackal/Hydrolib-soft.git
    GIT_TAG a8e8d50ad758489ed845ed1dc4a8717a38c4228b
)

FetchContent_MakeAvailable(Hydrolib)
