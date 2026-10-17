#pragma once

#include <stdint.h>

struct xsdp {
  char signature[8];

  uint8_t checksum;
  char oemid[6];
  uint8_t revision;
  uint32_t rsdt_address; // deprecated since version 2.0

  uint32_t length;
  uint64_t xsdt_address;
  uint8_t extended_checksum;
  uint8_t reserved[3];
} __attribute__((packed));

struct xsdt_header {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oem_id[6];
  char oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
};

struct xsdt {
  struct xsdt_header header;
  uint64_t tables[];
} __attribute__((packed));

void parse_acpi(const struct xsdp *xsdp);
