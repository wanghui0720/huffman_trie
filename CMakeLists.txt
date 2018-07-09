cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)

project(huffman_trie_test)

aux_source_directory(. DIR_SRCS)

add_executable(huffman_trie ./base/macros.h
                            ./net/tools/huffman_trie/bit_writer.h
                            ./net/tools/huffman_trie/bit_writer.cc
                            ./net/tools/huffman_trie/trie_entry.h
                            ./net/tools/huffman_trie/trie_entry.cc
                            ./net/tools/huffman_trie/huffman/huffman_builder.cc
                            ./net/tools/huffman_trie/huffman/huffman_builder.h
                            ./net/tools/huffman_trie/trie/trie_bit_buffer.cc
                            ./net/tools/huffman_trie/trie/trie_bit_buffer.h
                            ./net/tools/huffman_trie/trie/trie_writer.cc
                            ./net/tools/huffman_trie/trie/trie_writer.h
                            ./main.cc)

