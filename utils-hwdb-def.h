#ifndef UTILS_HWDB_DEF_H_
#define UTILS_HWDB_DEF_H_

#define HWDB_SIG { 'K', 'S', 'L', 'P', 'H', 'H', 'R', 'H' }

struct trie_header_f {
	uint8_t		signature[8];
	uint64_t	tool_version;
	uint64_t	file_size;
	uint64_t	header_size;
	uint64_t	node_size;
	uint64_t	child_entry_size;
	uint64_t	value_entry_size;
	uint64_t	nodes_root_off;
	uint64_t	nodes_len;
	uint64_t	strings_len;
} __attribute__((__packed__));

struct trie_node_f {
	uint64_t	prefix_off;
	uint8_t		children_count;
	uint8_t		padding[7];
	uint64_t	values_count;
} __attribute__((__packed__));

struct trie_child_entry_f {
	uint8_t		c;
	uint8_t		padding[7];
	uint64_t	child_off;
} __attribute__((__packed__));

struct trie_value_entry_f {
	uint64_t	key_off;
	uint64_t	value_off;
} __attribute__((__packed__));

#endif	/* UTILS_HWDB_DEF_H_ */
