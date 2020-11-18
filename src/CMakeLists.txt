add_library(
    dbsys20
    OBJECT
    ColumnStore.cpp
    RowStore.cpp
)
add_dependencies(dbsys20 Mutable)

add_executable(milestone1 milestone1.cpp $<TARGET_OBJECTS:dbsys20>)
target_link_libraries(milestone1 PRIVATE mutable)
