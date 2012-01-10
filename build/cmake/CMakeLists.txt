
file(
        GLOB_RECURSE
        source_files
        ../../*
)

add_library(sac ${source_files})

