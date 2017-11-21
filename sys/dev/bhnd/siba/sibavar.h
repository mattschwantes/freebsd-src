/*-
 * Copyright (c) 2015-2016 Landon Fuller <landon@landonf.org>
 * Copyright (c) 2017 The FreeBSD Foundation
 * All rights reserved.
 *
 * Portions of this software were developed by Landon Fuller
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 * 
 * $FreeBSD$
 */

#ifndef _SIBA_SIBAVAR_H_
#define _SIBA_SIBAVAR_H_

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/limits.h>

#include <machine/bus.h>
#include <sys/rman.h>

#include "siba.h"

/*
 * Internal definitions shared by siba(4) driver implementations.
 */

struct siba_addrspace;
struct siba_cfg_block;
struct siba_devinfo;
struct siba_core_id;

int			 siba_probe(device_t dev);
int			 siba_attach(device_t dev);
int			 siba_detach(device_t dev);
int			 siba_resume(device_t dev);
int			 siba_suspend(device_t dev);
u_int			 siba_get_intr_count(device_t dev, device_t child);
int			 siba_get_intr_ivec(device_t dev, device_t child,
			     u_int intr, u_int *ivec);

uint16_t		 siba_get_bhnd_mfgid(uint16_t ocp_vendor);

struct siba_core_id	 siba_parse_core_id(uint32_t idhigh, uint32_t idlow,
			     u_int core_idx, int unit);

int			 siba_add_children(device_t bus);

struct siba_devinfo	*siba_alloc_dinfo(device_t dev);
int			 siba_init_dinfo(device_t dev,
			     struct siba_devinfo *dinfo,
			     const struct siba_core_id *core_id);
void			 siba_free_dinfo(device_t dev, device_t child,
			     struct siba_devinfo *dinfo);

u_int			 siba_port_count(struct siba_core_id *core_id,
			     bhnd_port_type port_type);
bool			 siba_is_port_valid(struct siba_core_id *core_id,
			     bhnd_port_type port_type, u_int port);

u_int			 siba_port_region_count(
			     struct siba_core_id *core_id,
			     bhnd_port_type port_type, u_int port);

int			 siba_cfg_index(struct siba_core_id *core_id,
			     bhnd_port_type type, u_int port, u_int region,
			     u_int *cfgidx);

int			 siba_addrspace_index(struct siba_core_id *core_id,
			     bhnd_port_type type, u_int port, u_int region,
			     u_int *addridx);

u_int			 siba_addrspace_device_port(u_int addrspace);
u_int			 siba_addrspace_device_region(u_int addrspace);

u_int			 siba_cfg_agent_port(u_int cfg);
u_int			 siba_cfg_agent_region(u_int cfg);

struct siba_addrspace	*siba_find_addrspace(struct siba_devinfo *dinfo,
			     bhnd_port_type type, u_int port, u_int region);

struct siba_cfg_block	*siba_find_cfg_block(struct siba_devinfo *dinfo,
			     bhnd_port_type type, u_int port, u_int region);

int			 siba_append_dinfo_region(struct siba_devinfo *dinfo,
			     uint8_t sid, uint32_t base, uint32_t size,
			     uint32_t bus_reserved);

u_int			 siba_admatch_offset(uint8_t addrspace);
int			 siba_parse_admatch(uint32_t am, uint32_t *addr,
			     uint32_t *size);

int			 siba_write_target_state(device_t dev,
			     struct siba_devinfo *dinfo, bus_size_t reg,
			     uint32_t value, uint32_t mask);
int			 siba_wait_target_busy(device_t child,
			     struct siba_devinfo *dinfo, int usec);

							     
/* Sonics configuration register blocks */
#define	SIBA_CFG_NUM_2_2	1			/**< sonics <= 2.2 maps SIBA_CFG0. */
#define	SIBA_CFG_NUM_2_3	2			/**< sonics <= 2.3 maps SIBA_CFG0 and SIBA_CFG1 */
#define	SIBA_MAX_CFG		SIBA_CFG_NUM_2_3	/**< maximum number of supported config
							     register blocks */

#define	SIBA_CFG_RID_BASE	100			/**< base resource ID for SIBA_CFG* register allocations */
#define	SIBA_CFG_RID(_dinfo, _cfg)	\
	(SIBA_CFG_RID_BASE + (_cfg) +	\
	    (_dinfo->core_id.core_info.core_idx * SIBA_MAX_CFG))

/* Sonics/OCP address space mappings */
#define	SIBA_CORE_ADDRSPACE	0	/**< Address space mapping the primary
					     device registers */

#define	SIBA_MAX_ADDRSPACE	4	/**< Maximum number of Sonics/OCP
					  *  address space mappings for a
					  *  single core. */

/* bhnd(4) (port,region) representation of siba address space mappings */
#define	SIBA_MAX_PORT		2	/**< maximum number of advertised
					  *  bhnd(4) ports */

/** siba(4) address space descriptor */
struct siba_addrspace {
	uint32_t	sa_base;	/**< base address */
	uint32_t	sa_size;	/**< size */
	int		sa_rid;		/**< bus resource id */
	uint32_t	sa_bus_reserved;/**< number of bytes at high end of
					  *  address space reserved for the bus */
};

/** siba(4) config block descriptor */
struct siba_cfg_block {
	uint32_t	cb_base;	/**< base address */
	uint32_t	cb_size;	/**< size */
	int		cb_rid;		/**< bus resource id */
};

/** siba(4) backplane interrupt flag descriptor */
struct siba_intr {
	u_int		flag;	/**< backplane flag # */
	bool		mapped;	/**< if an irq has been mapped */
	int		rid;	/**< bus resource id, or -1 if unassigned */
	rman_res_t	irq;	/**< the mapped bus irq, if any */
};

/**
 * siba(4) per-core identification info.
 */
struct siba_core_id {
	struct bhnd_core_info	core_info;	/**< standard bhnd(4) core info */
	uint16_t		sonics_vendor;	/**< OCP vendor identifier used to generate
						  *  the JEDEC-106 bhnd(4) vendor identifier. */
	uint8_t			sonics_rev;	/**< sonics backplane revision code */
	uint8_t			num_addrspace;	/**< number of address ranges mapped to
						     this core. */
	uint8_t			num_cfg_blocks;	/**< number of Sonics configuration register
						     blocks mapped to the core's enumeration
						     space */
};

/**
 * siba(4) per-device info
 */
struct siba_devinfo {
	struct resource_list		 resources;			/**< per-core memory regions. */
	struct siba_core_id		 core_id;			/**< core identification info */
	struct siba_addrspace		 addrspace[SIBA_MAX_ADDRSPACE];	/**< memory map descriptors */
	struct siba_cfg_block		 cfg[SIBA_MAX_CFG];		/**< config block descriptors */
	struct siba_intr		 intr;				/**< interrupt flag descriptor, if any */
	bool				 intr_en;			/**< if true, core has an assigned interrupt flag */

	struct bhnd_resource		*cfg_res[SIBA_MAX_CFG];		/**< bus-mapped config block registers */
	int				 cfg_rid[SIBA_MAX_CFG];		/**< bus-mapped config block resource IDs */
	struct bhnd_core_pmu_info	*pmu_info;			/**< Bus-managed PMU state, or NULL */
};


/** siba(4) per-instance state */
struct siba_softc {
	struct bhnd_softc	bhnd_sc;	/**< bhnd state */
	device_t		dev;		/**< siba device */
};

#endif /* _SIBA_SIBAVAR_H_ */
