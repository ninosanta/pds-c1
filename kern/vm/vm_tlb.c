// vm_tlb.c: code for manipulating the tlb (including replacement)

#include "vm_tlb.h"
#include <lib.h>
#include <spl.h>
#include <spinlock.h>
#include <mips/tlb.h>

#include "vmstats.h"

static const unsigned char BYTE_BIT = 8;
static struct spinlock tlb_lock = SPINLOCK_INITIALIZER;
static struct tlb_map_t tlb_map;

tlb_report vmstats_report;

typedef struct tlb_map_t
{
  unsigned char *map; // Ogni bit identifica una cella della tlb
  unsigned char size; // Dimensione del vettore map
} tlb_map_t;

static int tlbmap_init(struct tlb_map_t *tlbmap);

/**
 * @brief Costruttore della struttura dati strcut tlb_map
 *
 * @param tlbmap
 * @return int
 */
static int tlbmap_init(struct tlb_map_t *tlbmap)
{
  tlbmap->size = NUM_TLB / (sizeof(unsigned char) * BYTE_BIT);
  if (tlbmap->size % BYTE_BIT)
    tlbmap->size++; // Aggiungo ulteriore spazio perche'occorre ulteriore spazio

  tlbmap->map = (unsigned char *)kmalloc(sizeof(unsigned char) * tlbmap->size);

  if (!tlbmap->map)
    return 1;

  return 0;
}

/**
 * @brief Seleziona una vittima all'interno della tlb applicando l'algoritmo round robin
 *
 * @return int
 */
static int tlb_get_rr_victim(void)
{
  int victim;
  static unsigned int next_victim = 0;
  victim = next_victim;
  next_victim = (next_victim + 1) % NUM_TLB;
  return victim;
}

static int vmtlb_searchIndex(void)
{
  int i, j;
  unsigned char shift = 1;

  spinlock_acquire(&tlb_lock);
  for (i = 0; i < tlb_map.size; i++)
  {
    // Analizza il byte alla ricerca di un bit a zero
    // 0xFF corrisponde a tutti i bit a 1. L'assenza di un bit comporta un risoltuato diveso da OxFF
    // Osservazioni: non credo sia utile avere l'operatore logico & 0xFF. Si sta applicando una maschera di soli uno
    if ((tlb_map.map[i] & 0xFF) == 0xFF)
      continue; // In questo blocco non abbiamo nessuna cella libera

    for (j = 0; j < BYTE_BIT; j++)
    {
      // Inversione dei bit per ricercare il bit libero (valore iniziale a 0).
      // Successivamente applica una maschera per trovare la posizione del bit libero
      // (con valore 1 in quando si e' applicato l'operatore not bit a bit)
      if (((~tlb_map.map[i]) & shift) == shift)
      {
        spinlock_release(&tlb_lock);
        return (BYTE_BIT * i) + j; // Ritorna la posizione del bit libero
      }
      else
        shift <<= 1;
    }
  }
  spinlock_release(&tlb_lock);
  return -1;
}

/**
 * @brief Construct a new vmtlb init object
 *
 */
int vmtlb_init(void)
{
  int i;

  if (tlbmap_init(&tlb_map))
    return 1; // Errore durante la fase di allocazione della memoria

  for (i = 0; i < tlb_map.size; i++)
  {
    tlb_map.map[i] = 0; // Tutte le celle della tlb sono inizializzate a zero
  }

  return 0;
}

void vmtlb_write(int *index, uint32_t ehi, uint32_t elo)
{
  int spl;

  if (*index == -1)
  {
    *index = vmtlb_searchIndex(); // Ricerca uno slot libero

    if (*index == -1)
    {
      *index = tlb_get_rr_victim();
      vmstats_report.tlb_faultReplacement++;
    }
    else
      vmstats_report.tlb_faultFree++;
  }
  else
    vmstats_report.tlb_faultFree++;

  spl = splhigh();
  tlb_write(ehi, elo, *index);

  // Inserisce il bit a uno alla posizione index
  // Avendo una suddivisione dei blocchi su 8 bit (unsigned char), occore posizionarsi nella
  // cella corretta e inserire il bit con valore 1 nella posizione corretta
  tlb_map.map[*index / BYTE_BIT] |= 1 << (*index % BYTE_BIT);

  splx(spl);
}

void vmtlb_clean(int index)
{
  int spl;

  spl = splhigh();

  tlb_write(TLBHI_INVALID(index), TLBLO_INVALID(), index);

  // Imposta il bit della cella da pulire a zero
  // Si ricerca il blocco i-esimo e si esegue l'operatore & bit a bit
  // tra il valore stesso e una sequenza di bit a 1 (0xff) e si inverte il bit
  // di interesse a 0 cosi' da rimuovere il valore 1
  tlb_map.map[index / BYTE_BIT] &= ~(1 << (index % BYTE_BIT));

  splx(spl);
}

/*
int tlb_init(void) {

}
int tlb_write_entry(void) { }
int tlb_replace_entry (void) { }

 */

/* cose probabilment einutili che ho scritto io
//modificare le entry della tlb in modo che segni quali pagine sono RW e RO
//se si tenta di accedere ad RO allora si generi un'eccezione senza far crashare il kernel
  --> dirty bit serve per segnare una entry come read only (0) o read/write (1)

//TLB init
//TLB destroy
//TLB_add_entry
//TLB_replace entry
//TLB_miss
//TLB_find
//TLB _destroy_entry

//TLB check process --> bisogn aassicurarsi che tutte le entri nella TLB siano appartenenti al processo
*/
