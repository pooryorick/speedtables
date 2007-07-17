/*
 * $Id$
 */

// Atomic word size, "cell_t".
typedef uint32_t cell_t;
#define CELLSIZE (sizeof (cell_t))

// Marker for the beginning of the list
#define MAP_MAGIC (((((('B' << 8) | 'O') << 8) | 'F') << 8) | 'H')
#define TRUE 1
#define FALSE 0

// TUNING

// How big a buffer of nulls to use when zeroing a file
#define NULBUFSIZE (1024L * 1024L)

// allocate a new reader block every READERS_PER_BLOCK readers
#define READERS_PER_BLOCK 64

// How many garbage entries to allocate at a time.
#define GARBAGE_POOL_SIZE 1024

// How long to leave garbage uncollected after it falls below the horizon
// (measured in lock cycles)
#define TWILIGHT_ZONE 1024

// How often to check for GC (1 in every LAZY_GC write cycles
#define LAZY_GC 10

// Sentinel for a reader that isn't holding any garbage
#define LOST_HORIZON 0

// internal errors
extern int shared_errno;
extern char *shared_errmsg[];
enum shared_error_e {
	SH_ERROR_0,
	SH_NEW_FILE,
	SH_PRIVATE_MEMORY,
	SH_MAP_FILE,
	SH_OPEN_FILE
};
extern void shared_perror(char *text);

// shared memory that is no longer needed but can't be freed until the
// last reader that was "live" when it was in use has released it
typedef struct _garbage {
    struct _garbage	*next;
    cell_t	         cycle;		// read cycle it's waiting on
    char		*block;		// address of block in shared mem
} garbage;

// Pool control block
//
// Pools have much lower overhead for small blocks, because you don't need
// a freelist and you don't need to merge blocks
typedef struct _pool {
    struct _pool	*next;		// next pool
    char		*start;		// start of pool
    int			 blocks;	// number of elements in pool
    int		 	 blocksize;	// size of each pool element
    int		 	 avail;		// number of chunks unallocated
    char		*brk;		// start of never-allocated space
    char		*freelist;      // first freed element
} pool;

// Symbol table, to pass addresses of internal structures to readers
typedef struct _symbol {
    volatile struct _symbol *next;
    volatile char	    *addr;
    char		     name[];
} symbol;

// Reader control block, containing READERS_PER_BLOCK reader records
// When a reader subscribes, it's handed the offset of its record
typedef struct _reader {
    cell_t		 pid;
    cell_t		 cycle;
} reader;

typedef struct _rblock {
    struct _rblock 	*next;		// offset of next reader block
    cell_t		 count;		// number of live readers in block
    reader		 readers[READERS_PER_BLOCK];
} reader_block;

// mapinfo->map points to this structure, at the front of the mapped file.
typedef struct _mapheader {
    cell_t           magic;		// Magic number, "initialised"
    cell_t           headersize;	// Size of this header
    cell_t           mapsize;		// Size of this map
    char	    *addr;		// Address mapped to
    volatile symbol *namelist;		// Internal symbol table
    cell_t           cycle;		// incremented every write
    reader_block     readers;		// advisory locks for readers
} mapheader;

// Freelist entry
typedef struct _freeblock {
    cell_t			  size;
    volatile struct _freeblock   *next;
    volatile struct _freeblock   *prev;
} freeblock;

// Busy block
typedef struct {
    cell_t			  size;
    char			  data[];
} busyblock;

typedef struct _mapinfo {
    struct _mapinfo	*next;
    volatile mapheader	*map;
    size_t		 size;
    int			 fd;
// server-only fields:
    pool		*pools;
    volatile freeblock	*freelist;
    garbage		*garbage;
    pool		*garbage_pool;
    cell_t		 horizon;
// client-only fields:
    volatile reader	*self;
} mapinfo;

int open_new(char *file, size_t size);
mapinfo *map_file(char *file, char *addr, size_t default_size);
int unmap_file(mapinfo *mapinfo);
void shminitmap(mapinfo *mapinfo);
pool *initpool(char *memory, size_t blocksize, int blocks);
pool *ckallocpool(size_t blocksize, int blocks);
int shmaddpool(mapinfo *map, size_t blocksize, int blocks);
char *palloc(pool *pool, size_t size);
void remove_from_freelist(mapinfo *mapinfo, volatile freeblock *block);
void insert_in_freelist(mapinfo *mapinfo, volatile freeblock *block);
char *_shmalloc(mapinfo *map, size_t size);
char *shmalloc(mapinfo *map, size_t size);
void shmfree(mapinfo *map, char *block);
int shmdepool(pool *pool, char *block);
void setfree(volatile freeblock *block, size_t size, int is_free);
int shmdealloc(mapinfo *mapinfo, char *data);
int write_lock(mapinfo *mapinfo);
void write_unlock(mapinfo *mapinfo);
volatile reader *pid2reader(volatile mapheader *map, int pid);
int read_lock(mapinfo *mapinfo);
void read_unlock(mapinfo *mapinfo);
void garbage_collect(mapinfo *mapinfo);
cell_t oldest_reader_cycle(mapinfo *mapinfo);

int add_symbol(mapinfo *mapinfo, char *name, char *value);
char *get_symbol(mapinfo *mapinfo, char *name);

// shift between the data inside a variable sized block, and the block itself
#define data2block(data) ((freeblock *)&((cell_t *)(data))[-1])
#define block2data(block) (((busyblock *)(block))->data)

#define prevsize(block) (((cell_t *)block)[-1])
#define nextblock(block) ((freeblock *)(((char *)block) + abs((block)->size)))
#define nextsize(block) (nextblock(block)->size)
#define prevblock(block) ((freeblock *)(((char *)block) - abs(prevsize(block))))

