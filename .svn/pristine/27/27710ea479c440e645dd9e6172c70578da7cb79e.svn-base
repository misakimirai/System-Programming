/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#include "page_table.h"
#include "physical_memory.h"
#include "virtual_memory.h"

/**
 * Helper function that can convert a virtual address into the index
 * of the page_directory_entry in a page directory
 */
static uint32_t get_vpn1(uint32_t virt_addr) {
    return virt_addr >> (NUM_VPN_BITS + NUM_OFFSET_BITS);
}

/**
 * Helper function that can convert a virtual address into the index
 * of the page_table_entry in a page directory
 */
static uint32_t get_vpn2(uint32_t virt_addr) {
    return virt_addr << (NUM_VPN_BITS) >> (NUM_VPN_BITS + NUM_OFFSET_BITS);
}

static uint32_t base_to_phys(addr32 base_addr) {
    return base_addr << NUM_OFFSET_BITS;
}

static uint32_t phys_to_base(addr32 addr) {
    return addr >> NUM_OFFSET_BITS;
}

pde_t *get_pde(struct process *proc, uint32_t virt_addr) {

    struct page_directory *directory = proc->paging_directory;
    uint32_t index = get_vpn1(virt_addr);

    return directory->entries + index;
}

pte_t *get_pte(struct process *proc, uint32_t virt_addr,
               bool allocate_page_table) {
    pde_t *directory_entry = get_pde(proc, virt_addr);

    if (!directory_entry->available) {
        if (!allocate_page_table) {
            return NULL;
        }
        struct page *meta_data = get_first_unused_page();
        if (!meta_data) {
            return NULL; // No more physical memory to make page tables ;(
        }
        addr32 addr_of_table = page_to_phys(meta_data);
        directory_entry->available = 1;
        // This page cannot be swapped out to disk

        meta_data->reserved = 1;
        directory_entry->base_addr = phys_to_base(addr_of_table);
    }

    addr32 base_addr_of_table_entry = directory_entry->base_addr;
    addr32 address_of_table_entry = base_to_phys(base_addr_of_table_entry);
    struct page_table *table =
        (struct page_table *)get_system_pointer(address_of_table_entry);

    uint32_t index_of_page_table = get_vpn2(virt_addr);
    struct page_table_entry *entry = table->entries + index_of_page_table;

    return entry;
}

void clear_pte(pte_t *page_table_entry) {
    *((uint32_t *)page_table_entry) = 0;
}

void clear_pde(pde_t *page_directory_entry) {
    *((uint32_t *)page_directory_entry) = 0;
}

bool page_table_in_use(pde_t *page_directory_entry) {
    struct page_table *table = (struct page_table *)get_system_pointer(
        base_to_phys(page_directory_entry->base_addr));
    for (uint32_t i = 0; i < NUM_ENTRIES; ++i) {
        if (table->entries[i].available) {
            return true;
        }
    }
    return false;
}
