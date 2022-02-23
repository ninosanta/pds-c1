/**
 * @file vm_tlb.c
 * @author your name (you@domain.com)
 * @brief code for manipulating the tlb (including replacement)
 * @version 0.1
 * @date 2022-02-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */


//Librerie
#include "vm_tlb.h"
#include <lib.h>
#include <spl.h>
#include <spinlock.h>
#include <mips/tlb.h>

#include "vmstats.h"

//Variabili Globali
static const unsigned char BYTE_BIT = 8;
static struct spinlock tlb_lock = SPINLOCK_INITIALIZER;
static struct tlb_map_t tlb_map;

typedef struct tlb_map_t
{
  unsigned char *map; // Ogni bit identifica una cella della tlb
  unsigned char size; // Dimensione del vettore map
} tlb_map_t;

/************************************************************
 *                                                          *
 * Implementazione delle funzioni                           *
 *                                                          *
 ************************************************************/

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

/**
 * @brief Analizza la TLB alla ricerca di un frame libero, se lo trova ritorna il suo indice
 *        Se non trova spazi liberi ritorna -1
 *
 * @return int
 */
static int vmtlb_searchIndex(void)
{
  int i, j;
  unsigned char shift = 1;

  spinlock_acquire(&tlb_lock);
  for (i = 0; i < tlb_map.size; i++)
  {
    // Analizza il byte alla ricerca di un bit a zero
    if ((tlb_map.map[i] & 0xFF) == 0xFF)
      continue; // In questo blocco non abbiamo nessuna cella libera

    // Nella cella i-esima abbiamo almeno un bit a 0.
    // Si ricerca la posizione del bit a 0 per ritornarlo al chiamante
    for (j = 0; j < BYTE_BIT; j++)
    {
      // Inversione dei bit per ricercare il bit libero (valore iniziale a 0).
      // Successivamente applica una maschera per trovare la posizione del bit libero
      // (con valore 1 in quando si e' applicato l'operatore not bit a bit)
      if (((~tlb_map.map[i]) & shift) == shift)
      {
        // Bit a 0 trovato
        spinlock_release(&tlb_lock);
        return (BYTE_BIT * i) + j; // Ritorna la posizione del bit libero
      }
      else
        // Bit a 1 non trovato
        shift <<= 1; // Sposta il bit a 1. 
                     // Il valore della variabile sarà compreso tra 0x01 e 0x80
    }
  }
  spinlock_release(&tlb_lock);
  return -1;
}

/**
 * @brief Crea un nuovo oggetto tlb e lo inizializza
 *
 * @return int 
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

/**
 * @brief Se l'indice non è ancora stato trovato: cerca una entry vuota, se non c'è trova una entry da sostituire
 *         Una volta selezionato l'indice, fa una tlb_write per aggiungere la nuova entry
 *
 * @param int, uint32_t, uint32_t
 * @return void
 */
void vmtlb_write(int *index, uint32_t ehi, uint32_t elo)
{
  int spl;

  if (*index == -1)
  {
    *index = vmtlb_searchIndex(); // Ricerca uno slot libero

    if (*index == -1)
    {
      //Se non trovato seleziono una vittima da rimpiazzare
      *index = tlb_get_rr_victim();
      vmstats_report_tlb_faultReplacement_increment(); 
    }
    else
      vmstats_report_tlb_faultFree_increment();
  }
  else
    vmstats_report_tlb_faultFree_increment();

  spl = splhigh();

  //Aggiunge la entry in tlb
  tlb_write(ehi, elo, *index); // Funzione mips

  // Inserisce il bit a uno alla posizione index
  tlb_map.map[*index / BYTE_BIT] |= 1 << (*index % BYTE_BIT);
  
  splx(spl);
}

/**
 * @brief Dato l'indice, invalida la entry corrispondente
 *
 * @param int
 */
void vmtlb_clean(int index)
{
  int spl;

  spl = splhigh();

  tlb_write(TLBHI_INVALID(index), TLBLO_INVALID(), index); // Funzione mips

  // Imposta il bit della cella da pulire a zero
  tlb_map.map[index / BYTE_BIT] &= ~(1 << (index % BYTE_BIT)); // Si ricerca il blocco i-esimo e si esegue l'operatore & bit a bit
                                                          // tra il valore stesso e una sequenza di bit a 1 (0xff) e si inverte il bit
                                                          // di interesse a 0 cosi' da rimuovere il valore 1
  splx(spl);
}

