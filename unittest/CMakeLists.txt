add_executable(
    unittest
    main.cpp
    ColumnStoreTest.cpp
    RowStoreTest.cpp
)
target_link_libraries(unittest mutable $<TARGET_OBJECTS:dbsys20>)
