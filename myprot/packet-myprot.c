// Dissector.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "config.h"
#include <epan/packet.h>

#define myprot_port 1234


static int myprot = -1;
static int hf_myprot_pdu_type = -1;
static gint ett_myprot = -1;
static int hf_myprot_flags = -1;
static int hf_myprot_sequenceno = -1;
static int hf_myprot_initialip = -1;

static const value_string packettypenames[] = {
	{ 1, "Response" },
	{ 0, "Request" }
};
static const value_string packetflags[] = {
	{ 1, "High" },
	{ 0, "Low" }
};


static int dissect_myprot(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree _U_, void *data _U_)
{
	gint offset = 0;

	col_set_str(pinfo->cinfo, COL_PROTOCOL, "MyProt");
	/* Clear out stuff in the info column */
	col_clear(pinfo->cinfo, COL_INFO);

	proto_item *ti = proto_tree_add_item(tree, myprot, tvb, 0, -1, ENC_NA);
	proto_tree *myprot_tree = proto_item_add_subtree(ti, ett_myprot);
	proto_tree_add_item(myprot_tree, hf_myprot_sequenceno, tvb, offset, 1, ENC_BIG_ENDIAN);
	offset += 1;
	proto_tree_add_item(myprot_tree, hf_myprot_pdu_type, tvb, offset, 1, ENC_BIG_ENDIAN);
	offset += 1;
	proto_tree_add_item(myprot_tree, hf_myprot_flags, tvb, offset, 1, ENC_BIG_ENDIAN);


	return tvb_captured_length(tvb);
}

void proto_register_myprot(void)
{
	static hf_register_info hf[] = {
		{ &hf_myprot_sequenceno,
			{ "MyProt PDU Sequence Number", "myprot.seqn",
			FT_UINT16, BASE_DEC,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_myprot_pdu_type, 
			{ "MyProt PDU Type", "myprot.type",
			FT_UINT8, BASE_DEC,
			VALS(packettypenames), 0X0,
			NULL, HFILL}
		},
		{ &hf_myprot_flags,
			{ "MyProt PDU Flags", "myprot.flags",
			FT_UINT8, BASE_DEC,
			VALS(packetflags), 0x0,
			NULL, HFILL }
		},
	};

	/*Setup protocol subtree array*/
	static gint *ett[] = {
		&ett_myprot
	};

	myprot = proto_register_protocol(
		"MyProt Protocol", /*name */
		"MyProt", /*short name */
		"myprot" /*abbrev*/
		);
	proto_register_field_array(myprot, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));
}

void proto_reg_handoff_myprot(void)
{
	static dissector_handle_t myprot_handle;

	myprot_handle = create_dissector_handle(dissect_myprot, myprot);//create a handler for our protocol and give it a routine to be called to do the dissection
	dissector_add_uint("udp.port", myprot_port, myprot_handle); /* when we have UDP traffic on myprot_port, the main program will know to call us*/
}

