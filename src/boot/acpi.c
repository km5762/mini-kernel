#include "boot/acpi.h"
#include <stddef.h>

void parse_acpi(const struct xsdp *xsdp) {
  const struct xsdt *xsdt = (const struct xsdt *)xsdp->xsdt_address;
  const size_t tables =
      (xsdt->header.length - sizeof(xsdt->header)) / sizeof(uint64_t);

  for (size_t i = 0; i < tables; ++i) {
    const struct xsdt_header *header =
        (const struct xsdt_header *)xsdt->tables[i];

    if (header->signature == "MADT") {
    }
  }
}
