#include "./net/tools/huffman_trie/huffman/huffman_builder.h"
#include "./net/tools/huffman_trie/trie/trie_bit_buffer.h"
#include "./net/tools/huffman_trie/trie/trie_writer.h"
#include <iostream>

using namespace std;
using net::huffman_trie::HuffmanRepresentationTable;
using net::huffman_trie::HuffmanBuilder;
using net::huffman_trie::TrieWriter;

static const char kNewLine[] = "\n";
static const char kIndent[] = "  ";

struct TestEntry {
  std::string host;
  std::string style;
};

class TestTrieEntry : public net::huffman_trie::TrieEntry {
 public:
  explicit TestTrieEntry(
      const net::huffman_trie::HuffmanRepresentationTable& huffman_table,
      net::huffman_trie::HuffmanBuilder* huffman_builder,
      TestEntry* entry);
  ~TestTrieEntry() override;

  // huffman_trie::TrieEntry:
  std::string name() const override;
  bool WriteEntry(net::huffman_trie::TrieBitBuffer* writer) const override;

 private:
  const net::huffman_trie::HuffmanRepresentationTable& huffman_table_;
  net::huffman_trie::HuffmanBuilder* huffman_builder_;
  TestEntry* entry_;
};

using TestEntries = std::vector<TestEntry*>;

TestTrieEntry::TestTrieEntry(
    const net::huffman_trie::HuffmanRepresentationTable& huffman_table,
    net::huffman_trie::HuffmanBuilder* huffman_builder,
    TestEntry* entry)
    : huffman_table_(huffman_table),
      huffman_builder_(huffman_builder),
      entry_(entry) {}

TestTrieEntry::~TestTrieEntry() {}

std::string TestTrieEntry::name() const {
  return entry_->host;
}

bool TestTrieEntry::WriteEntry(
    net::huffman_trie::TrieBitBuffer* writer) const {
  if (entry_->host == entry_->style) {
    writer->WriteBit(1);
    return true;
  }
  writer->WriteBit(0);

  std::string style = entry_->style;

  for (const auto& c : style) {
    writer->WriteChar(c, huffman_table_, huffman_builder_);
  }
  writer->WriteChar(net::huffman_trie::kEndOfTableValue, huffman_table_,
                    huffman_builder_);
  return true;
}

// Formats the bytes in |bytes| as an C++ array initializer and returns the
// resulting string.
std::string FormatVectorAsArray(const std::vector<uint8_t>& bytes) {
  std::string output = "{";
  output.append(kNewLine);
  output.append(kIndent);
  output.append(kIndent);

  size_t bytes_on_current_line = 0;

  for (size_t i = 0; i < bytes.size(); ++i) {
    char tmp[4];
    sprintf(tmp, "0x%02x,", bytes[i]);
    output.append(std::string(tmp));

    bytes_on_current_line++;
    if (bytes_on_current_line >= 12 && (i + 1) < bytes.size()) {
      output.append(kNewLine);
      output.append(kIndent);
      output.append(kIndent);

      bytes_on_current_line = 0;
    } else if ((i + 1) < bytes.size()) {
      output.append(" ");
    }
  }

  output.append(kNewLine);
  output.append("}");

  return output;
}

HuffmanRepresentationTable ApproximateHuffman(const TestEntries entries) {
  HuffmanBuilder huffman_builder;
  for (const auto& entry : entries) {
    for (const auto& c : entry->host) {
      huffman_builder.RecordUsage(c);
    } 
    for (const auto& c : entry->style) {
      huffman_builder.RecordUsage(c);
    }
    huffman_builder.RecordUsage(net::huffman_trie::kTerminalValue);
    huffman_builder.RecordUsage(net::huffman_trie::kEndOfTableValue);
  }
  return huffman_builder.ToTable(); 
}

int main() {
  TestEntries entries;

  auto entry1 = new TestEntry();
  entry1->host = "www.sogo.com";
  entry1->style = "none";

  auto entry2 = new TestEntry();
  entry2->host = "www.sogou.com";
  entry2->style = "none";
  
  entries.push_back(entry1);
  entries.push_back(entry2);
 
  HuffmanRepresentationTable approximate_table = ApproximateHuffman(entries); 
  HuffmanBuilder huffman_builder;

  // Create trie entries for the first pass.
  std::vector<std::unique_ptr<TestTrieEntry>> trie_entries;
  std::vector<net::huffman_trie::TrieEntry*> raw_trie_entries;
  for (const auto& entry : entries) {
    auto trie_entry = std::make_unique<TestTrieEntry>(
        approximate_table, &huffman_builder, entry);
    raw_trie_entries.push_back(trie_entry.get());
    trie_entries.push_back(std::move(trie_entry));
  }

  TrieWriter writer(approximate_table, &huffman_builder);
  uint32_t root_position;
  if (!writer.WriteEntries(raw_trie_entries, &root_position)) {
    std::cout<< "write entry error occur" << std::endl;
    return 0;
  }

  HuffmanRepresentationTable optimal_table = huffman_builder.ToTable();
  TrieWriter new_writer(optimal_table, &huffman_builder);

  // Create trie entries using the optimal table for the second pass.
  raw_trie_entries.clear();
  trie_entries.clear();
  for (const auto& entry : entries) {
    auto trie_entry = std::make_unique<TestTrieEntry>(
        optimal_table, &huffman_builder, entry);
    raw_trie_entries.push_back(trie_entry.get());
    trie_entries.push_back(std::move(trie_entry));
  }

  if (!new_writer.WriteEntries(raw_trie_entries, &root_position)) {
    std::cout<< "write entry error occur" << std::endl;
    return 0;
  }

  uint32_t new_length = new_writer.position();
  std::vector<uint8_t> huffman_tree = huffman_builder.ToVector();
  new_writer.Flush();

  std::string output = FormatVectorAsArray(huffman_tree);
  std::cout << " test " <<  output << std::endl;

  // TODO: huffman_trie decode
   
  return 1;
}
