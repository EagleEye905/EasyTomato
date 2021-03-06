/*
 * ip_nat_proto_gre.c - Version 1.2
 *
 * NAT protocol helper module for GRE.
 *
 * GRE is a generic encapsulation protocol, which is generally not very
 * suited for NAT, as it has no protocol-specific part as port numbers.
 *
 * It has an optional key field, which may help us distinguishing two 
 * connections between the same two hosts.
 *
 * GRE is defined in RFC 1701 and RFC 1702, as well as RFC 2784 
 *
 * PPTP is built on top of a modified version of GRE, and has a mandatory
 * field called "CallID", which serves us for the same purpose as the key
 * field in plain GRE.
 *
 * Documentation about PPTP can be found in RFC 2637
 *
 * (C) 2000-2003 by Harald Welte <laforge@gnumonks.org>
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_nat_protocol.h>
#include <linux/netfilter_ipv4/ip_conntrack_proto_gre.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harald Welte <laforge@gnumonks.org>");
MODULE_DESCRIPTION("Netfilter NAT protocol helper module for GRE");

#if 0
#define DEBUGP(format, args...) printk(KERN_DEBUG "%s:%s: " format, __FILE__, __FUNCTION__, ## args)
#else
#define DEBUGP(x, args...)
#endif

/* is key in given range between min and max */
static int
gre_in_range(const struct ip_conntrack_tuple *tuple,
	     enum ip_nat_manip_type maniptype,
	     const union ip_conntrack_manip_proto *min,
	     const union ip_conntrack_manip_proto *max)
{
	u_int16_t key;

	if (maniptype == IP_NAT_MANIP_SRC)
		key = tuple->src.u.gre.key;
	else
		key = tuple->dst.u.gre.key;

	return ntohs(key) >= ntohs(min->gre.key)
		&& ntohs(key) <= ntohs(max->gre.key);
}

/* generate unique tuple ... */
static int 
gre_unique_tuple(struct ip_conntrack_tuple *tuple,
		 const struct ip_nat_range *range,
		 enum ip_nat_manip_type maniptype,
		 const struct ip_conntrack *conntrack)
{
	unsigned int min, i, range_size;
	u_int16_t key = 0, *keyptr;

	/* If there is no master conntrack we are not PPTP,
	   do not change tuples */
	if (!conntrack->master)
		return 0;

	if (maniptype == IP_NAT_MANIP_SRC)
		keyptr = &tuple->src.u.gre.key;
	else
		keyptr = &tuple->dst.u.gre.key;

	if (!(range->flags & IP_NAT_RANGE_PROTO_SPECIFIED)) {

		DEBUGP("%p: NATing GRE PPTP\n", conntrack);
		min = 1;
		range_size = 0xffff;

	} else {
		min = ntohs(range->min.gre.key);
		range_size = ntohs(range->max.gre.key) - min + 1;
	}

	DEBUGP("min = %u, range_size = %u\n", min, range_size); 

	for (i = 0; i < range_size; i++, key++) {
		*keyptr = htons(min + key % range_size);
		if (!ip_nat_used_tuple(tuple, conntrack))
			return 1;
	}

	DEBUGP("%p: no NAT mapping\n", conntrack);

	return 0;
}

/* manipulate a GRE packet according to maniptype */
static void 
gre_manip_pkt(struct iphdr *iph, size_t len, 
	      const struct ip_conntrack_manip *manip,
	      enum ip_nat_manip_type maniptype)
{
	struct gre_hdr *greh = (struct gre_hdr *)((u_int32_t *)iph+iph->ihl);
	struct gre_hdr_pptp *pgreh = (struct gre_hdr_pptp *) greh;

	/* we only have destination manip of a packet, since 'source key' 
	 * is not present in the packet itself */
	if (maniptype == IP_NAT_MANIP_DST) {
		/* key manipulation is always dest */
		switch (greh->version) {
		case GRE_VERSION_1701:
			/* We do not currently NAT any GREv0 packets.
			 * Try to behave like "nf_nat_proto_unknown" */
			break;
		case GRE_VERSION_PPTP:
			DEBUGP("call_id -> 0x%04x\n", 
				ntohs(manip->u.gre.key));
			pgreh->call_id = manip->u.gre.key;
			break;
		default:
			DEBUGP("can't nat unknown GRE version\n");
			break;
		}
	}
}

/* print out a nat tuple */
static unsigned int 
gre_print(char *buffer, 
	  const struct ip_conntrack_tuple *match,
	  const struct ip_conntrack_tuple *mask)
{
	unsigned int len = 0;

	if (mask->src.u.gre.key)
		len += sprintf(buffer + len, "srckey=0x%x ", 
				ntohs(match->src.u.gre.key));

	if (mask->dst.u.gre.key)
		len += sprintf(buffer + len, "dstkey=0x%x ",
				ntohs(match->src.u.gre.key));

	return len;
}

/* print a range of keys */
static unsigned int 
gre_print_range(char *buffer, const struct ip_nat_range *range)
{
	if (range->min.gre.key != 0 
	    || range->max.gre.key != 0xFFFF) {
		if (range->min.gre.key == range->max.gre.key)
			return sprintf(buffer, "key 0x%x ",
					ntohs(range->min.gre.key));
		else
			return sprintf(buffer, "keys 0x%u-0x%u ",
					ntohs(range->min.gre.key),
					ntohs(range->max.gre.key));
	} else
		return 0;
}

/* nat helper struct */
static struct ip_nat_protocol gre = 
	{ { NULL, NULL }, "GRE", IPPROTO_GRE,
	  gre_manip_pkt,
	  gre_in_range,
	  gre_unique_tuple,
	  gre_print,
	  gre_print_range 
	};
				  
static int __init init(void)
{
        if (ip_nat_protocol_register(&gre))
                return -EIO;

        return 0;
}

static void __exit fini(void)
{
        ip_nat_protocol_unregister(&gre);
}

module_init(init);
module_exit(fini);
