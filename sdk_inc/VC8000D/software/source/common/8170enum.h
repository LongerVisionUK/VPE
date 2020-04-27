HWIF_DEC_PIC_INF, HWIF_DEC_TILE_INT, HWIF_DEC_LINE_CNT_INT,
    HWIF_DEC_EXT_TIMEOUT_INT, HWIF_DEC_NO_SLICE_INT, HWIF_DEC_LAST_SLICE_INT,
    HWIF_DEC_TIMEOUT, HWIF_DEC_SLICE_INT, HWIF_DEC_ERROR_INT, HWIF_DEC_ASO_INT,
    HWIF_DEC_BUFFER_INT, HWIF_DEC_BUS_INT, HWIF_DEC_RDY_INT, HWIF_DEC_ABORT_INT,
    HWIF_DEC_IRQ, HWIF_DEC_TILE_INT_E, HWIF_DEC_SELF_RESET_DIS,
    HWIF_DEC_ABORT_E, HWIF_DEC_IRQ_DIS, HWIF_DEC_TIMEOUT_SOURCE,
    HWIF_DEC_BUS_INT_DIS, HWIF_DEC_STRM_CORRUPTED, HWIF_DEC_E,
    HWIF_DEC_STRM_SWAP, HWIF_DEC_PIC_SWAP, HWIF_DEC_DIRMV_SWAP,
    HWIF_TILED_MODE_MSB, HWIF_DEC_TAB_SWAP, HWIF_DEC_CLK_GATE_E,
    HWIF_TILED_MODE_LSB, HWIF_DEC_MODE, HWIF_SKIP_MODE, HWIF_DIVX3_E,
    HWIF_RLC_MODE_E, HWIF_PIC_INTERLACE_E, HWIF_PIC_FIELDMODE_E, HWIF_PIC_B_E,
    HWIF_PIC_INTER_E, HWIF_PIC_TOPFIELD_E, HWIF_FWD_INTERLACE_E,
    HWIF_SORENSON_E, HWIF_REF_TOPFIELD_E, HWIF_DEC_OUT_DIS, HWIF_FILTERING_DIS,
    HWIF_WEBP_E, HWIF_MVC_E, HWIF_PIC_FIXED_QUANT, HWIF_WRITE_MVS_E,
    HWIF_REFTOPFIRST_E, HWIF_SEQ_MBAFF_E, HWIF_PICORD_COUNT_E,
    HWIF_DEC_OUT_EC_BYPASS, HWIF_APF_ONE_PID, HWIF_L2_SHAPER_E,
    HWIF_BUFFER_EMPTY_INT_E, HWIF_BLOCK_BUFFER_MODE_E, HWIF_LAST_BUFFER_E,
    HWIF_PIC_MB_WIDTH, HWIF_MB_WIDTH_OFF, HWIF_PIC_MB_HEIGHT_P,
    HWIF_MB_HEIGHT_OFF, HWIF_PIC_WIDTH_IN_CBS, HWIF_PIC_HEIGHT_IN_CBS,
    HWIF_ALT_SCAN_E, HWIF_TOPFIELDFIRST_E, HWIF_REF_FRAMES, HWIF_PIC_MB_W_EXT,
    HWIF_PIC_MB_H_EXT, HWIF_PIC_REFER_FLAG, HWIF_STRM_START_BIT,
    HWIF_SCALING_LIST_E, HWIF_TYPE1_QUANT_E, HWIF_CH_QP_OFFSET,
    HWIF_CH_QP_OFFSET2, HWIF_SIGN_DATA_HIDE, HWIF_TEMPOR_MVP_E,
    HWIF_MAX_CU_QPD_DEPTH, HWIF_CU_QPD_E, HWIF_FIELDPIC_FLAG_E,
    HWIF_INTRADC_VLC_THR, HWIF_VOP_TIME_INCR, HWIF_DQ_PROFILE, HWIF_DQBI_LEVEL,
    HWIF_RANGE_RED_FRM_E, HWIF_FAST_UVMC_E, HWIF_TRANSDCTAB, HWIF_TRANSACFRM,
    HWIF_TRANSACFRM2, HWIF_MB_MODE_TAB, HWIF_MVTAB, HWIF_CBPTAB,
    HWIF_2MV_BLK_PAT_TAB, HWIF_4MV_BLK_PAT_TAB, HWIF_QSCALE_TYPE, HWIF_CON_MV_E,
    HWIF_INTRA_DC_PREC, HWIF_INTRA_VLC_TAB, HWIF_FRAME_PRED_DCT,
    HWIF_JPEG_QTABLES, HWIF_JPEG_MODE, HWIF_JPEG_FILRIGHT_E,
    HWIF_JPEG_STREAM_ALL, HWIF_CR_AC_VLCTABLE, HWIF_CB_AC_VLCTABLE,
    HWIF_CR_DC_VLCTABLE, HWIF_CB_DC_VLCTABLE, HWIF_CR_DC_VLCTABLE3,
    HWIF_CB_DC_VLCTABLE3, HWIF_STRM1_START_BIT, HWIF_HUFFMAN_E,
    HWIF_MULTISTREAM_E, HWIF_BOOLEAN_VALUE, HWIF_BOOLEAN_RANGE,
    HWIF_ALPHA_OFFSET, HWIF_BETA_OFFSET, HWIF_STREAM_LEN, HWIF_CABAC_E,
    HWIF_CABAC_INIT_PRESENT, HWIF_BLACKWHITE_E, HWIF_DIR_8X8_INFER_E,
    HWIF_WEIGHT_PRED_E, HWIF_WEIGHT_BIPR_IDC, HWIF_AVS_H264_H_EXT,
    HWIF_CH_8PIX_ILEAV_E, HWIF_FRAMENUM_LEN, HWIF_FRAMENUM, HWIF_BITPLANE0_E,
    HWIF_BITPLANE1_E, HWIF_BITPLANE2_E, HWIF_ALT_PQUANT, HWIF_DQ_EDGES,
    HWIF_TTMBF, HWIF_PQINDEX, HWIF_VC1_HEIGHT_EXT, HWIF_BILIN_MC_E,
    HWIF_UNIQP_E, HWIF_HALFQP_E, HWIF_TTFRM, HWIF_2ND_BYTE_EMUL_E,
    HWIF_DQUANT_E, HWIF_VC1_ADV_E, HWIF_PJPEG_FILDOWN_E, HWIF_PJPEG_WDIV8,
    HWIF_PJPEG_HDIV8, HWIF_PJPEG_AH, HWIF_PJPEG_AL, HWIF_PJPEG_SS,
    HWIF_PJPEG_SE, HWIF_DCT1_START_BIT, HWIF_DCT2_START_BIT, HWIF_CH_MV_RES,
    HWIF_INIT_DC_MATCH0, HWIF_INIT_DC_MATCH1, HWIF_VP7_VERSION,
    HWIF_FILT_SLICE_BORDER, HWIF_FILT_TILE_BORDER, HWIF_ASYM_PRED_E, HWIF_SAO_E,
    HWIF_PCM_FILT_DISABLE, HWIF_SLICE_CHQP_FLAG, HWIF_DEPEND_SLICE_E,
    HWIF_FILT_OVERRIDE_E, HWIF_STRONG_SMOOTH_E, HWIF_FILT_OFFSET_BETA,
    HWIF_FILT_OFFSET_TC, HWIF_SLICE_HDR_EXT_E, HWIF_SLICE_HDR_EBITS,
    HWIF_CONST_INTRA_E, HWIF_FILT_CTRL_PRES, HWIF_RDPIC_CNT_PRES,
    HWIF_8X8TRANS_FLAG_E, HWIF_REFPIC_MK_LEN, HWIF_IDR_PIC_E, HWIF_IDR_PIC_ID,
    HWIF_PCM_BITDEPTH_Y, HWIF_PCM_BITDEPTH_C, HWIF_BIT_DEPTH_Y_MINUS8,
    HWIF_BIT_DEPTH_C_MINUS8, HWIF_MV_SCALEFACTOR, HWIF_REF_DIST_FWD,
    HWIF_REF_DIST_BWD, HWIF_LOOP_FILT_LIMIT, HWIF_VARIANCE_TEST_E,
    HWIF_MV_THRESHOLD, HWIF_VAR_THRESHOLD, HWIF_DIVX_IDCT_E,
    HWIF_DIVX3_SLICE_SIZE, HWIF_PJPEG_REST_FREQ, HWIF_RV_PROFILE,
    HWIF_RV_OSV_QUANT, HWIF_RV_FWD_SCALE, HWIF_RV_BWD_SCALE, HWIF_INIT_DC_COMP0,
    HWIF_INIT_DC_COMP1, HWIF_PPS_ID, HWIF_REFIDX1_ACTIVE, HWIF_REFIDX0_ACTIVE,
    HWIF_HDR_SKIP_LENGTH, HWIF_POC_LENGTH, HWIF_ICOMP0_E, HWIF_ISCALE0,
    HWIF_ISHIFT0, HWIF_STREAM1_LEN, HWIF_PIC_SLICE_AM, HWIF_COEFFS_PART_AM,
    HWIF_PINIT_RLIST_F9, HWIF_PINIT_RLIST_F8, HWIF_PINIT_RLIST_F7,
    HWIF_PINIT_RLIST_F6, HWIF_PINIT_RLIST_F5, HWIF_PINIT_RLIST_F4,
    HWIF_ICOMP1_E, HWIF_ISCALE1, HWIF_ISHIFT1, HWIF_INIT_QP_V0,
    HWIF_NUM_TILE_COLS_V0, HWIF_NUM_TILE_COLS, HWIF_NUM_TILE_COLS_8K,
    HWIF_NUM_TILE_ROWS_V0, HWIF_NUM_TILE_ROWS, HWIF_NUM_TILE_ROWS_8K,
    HWIF_TILE_ENABLE, HWIF_ENTR_CODE_SYNCH_E, HWIF_PINIT_RLIST_F15,
    HWIF_PINIT_RLIST_F14, HWIF_PINIT_RLIST_F13, HWIF_PINIT_RLIST_F12,
    HWIF_PINIT_RLIST_F11, HWIF_PINIT_RLIST_F10, HWIF_ICOMP2_E, HWIF_ISCALE2,
    HWIF_ISHIFT2, HWIF_DCT3_START_BIT, HWIF_DCT4_START_BIT, HWIF_DCT5_START_BIT,
    HWIF_DCT6_START_BIT, HWIF_DCT7_START_BIT, HWIF_TRANSFORM_MODE,
    HWIF_MCOMP_FILT_TYPE, HWIF_HIGH_PREC_MV_E, HWIF_COMP_PRED_MODE,
    HWIF_SYNC_MARKER_E, HWIF_MIN_CB_SIZE, HWIF_MAX_CB_SIZE, HWIF_MIN_PCM_SIZE,
    HWIF_MAX_PCM_SIZE, HWIF_PCM_E, HWIF_TRANSFORM_SKIP_E, HWIF_TRANSQ_BYPASS_E,
    HWIF_REFPICLIST_MOD_E, HWIF_START_CODE_E, HWIF_INIT_QP, HWIF_MIN_TRB_SIZE,
    HWIF_MAX_TRB_SIZE, HWIF_MAX_INTRA_HIERDEPTH, HWIF_MAX_INTER_HIERDEPTH,
    HWIF_PARALLEL_MERGE, HWIF_QP_DELTA_Y_DC, HWIF_QP_DELTA_CH_DC,
    HWIF_QP_DELTA_CH_AC, HWIF_LAST_SIGN_BIAS, HWIF_LOSSLESS_E,
    HWIF_COMP_PRED_VAR_REF1, HWIF_COMP_PRED_VAR_REF0, HWIF_COMP_PRED_FIXED_REF,
    HWIF_SEGMENT_TEMP_UPD_E, HWIF_SEGMENT_UPD_E, HWIF_SEGMENT_E,
    HWIF_INIT_RLIST_B2, HWIF_INIT_RLIST_F2, HWIF_INIT_RLIST_B1,
    HWIF_INIT_RLIST_F1, HWIF_INIT_RLIST_B0, HWIF_INIT_RLIST_F0, HWIF_FILT_LEVEL,
    HWIF_REFPIC_SEG0, HWIF_SKIP_SEG0, HWIF_FILT_LEVEL_SEG0, HWIF_QUANT_SEG0,
    HWIF_JPEG_SLICE_H, HWIF_INIT_RLIST_B5, HWIF_INIT_RLIST_F5,
    HWIF_INIT_RLIST_B4, HWIF_INIT_RLIST_F4, HWIF_INIT_RLIST_B3,
    HWIF_INIT_RLIST_F3, HWIF_REFPIC_SEG1, HWIF_SKIP_SEG1, HWIF_FILT_LEVEL_SEG1,
    HWIF_QUANT_SEG1, HWIF_AC1_CODE6_CNT, HWIF_AC1_CODE5_CNT, HWIF_AC1_CODE4_CNT,
    HWIF_AC1_CODE3_CNT, HWIF_AC1_CODE2_CNT, HWIF_AC1_CODE1_CNT,
    HWIF_INIT_RLIST_B8, HWIF_INIT_RLIST_F8, HWIF_INIT_RLIST_B7,
    HWIF_INIT_RLIST_F7, HWIF_INIT_RLIST_B6, HWIF_INIT_RLIST_F6,
    HWIF_REFPIC_SEG2, HWIF_SKIP_SEG2, HWIF_FILT_LEVEL_SEG2, HWIF_QUANT_SEG2,
    HWIF_AC1_CODE10_CNT, HWIF_AC1_CODE9_CNT, HWIF_AC1_CODE8_CNT,
    HWIF_AC1_CODE7_CNT, HWIF_INIT_RLIST_B11, HWIF_INIT_RLIST_F11,
    HWIF_INIT_RLIST_B10, HWIF_INIT_RLIST_F10, HWIF_INIT_RLIST_B9,
    HWIF_INIT_RLIST_F9, HWIF_REFPIC_SEG3, HWIF_SKIP_SEG3, HWIF_FILT_LEVEL_SEG3,
    HWIF_QUANT_SEG3, HWIF_PIC_HEADER_LEN, HWIF_PIC_4MV_E, HWIF_RANGE_RED_REF_E,
    HWIF_VC1_DIFMV_RANGE, HWIF_MV_RANGE, HWIF_OVERLAP_E, HWIF_OVERLAP_METHOD,
    HWIF_ALT_SCAN_FLAG_E, HWIF_FCODE_FWD_HOR, HWIF_FCODE_FWD_VER,
    HWIF_FCODE_BWD_HOR, HWIF_FCODE_BWD_VER, HWIF_MV_ACCURACY_FWD,
    HWIF_MV_ACCURACY_BWD, HWIF_MPEG4_VC1_RC, HWIF_PREV_ANC_TYPE,
    HWIF_AC1_CODE14_CNT, HWIF_AC1_CODE13_CNT, HWIF_AC1_CODE12_CNT,
    HWIF_AC1_CODE11_CNT, HWIF_INIT_RLIST_B14, HWIF_INIT_RLIST_F14,
    HWIF_INIT_RLIST_B13, HWIF_INIT_RLIST_F13, HWIF_INIT_RLIST_B12,
    HWIF_INIT_RLIST_F12, HWIF_REFPIC_SEG4, HWIF_SKIP_SEG4, HWIF_FILT_LEVEL_SEG4,
    HWIF_QUANT_SEG4, HWIF_TRB_PER_TRD_D0, HWIF_ICOMP3_E, HWIF_ISCALE3,
    HWIF_ISHIFT3, HWIF_AC2_CODE4_CNT, HWIF_AC2_CODE3_CNT, HWIF_AC2_CODE2_CNT,
    HWIF_AC2_CODE1_CNT, HWIF_AC1_CODE16_CNT, HWIF_AC1_CODE15_CNT,
    HWIF_SCAN_MAP_1, HWIF_SCAN_MAP_2, HWIF_SCAN_MAP_3, HWIF_SCAN_MAP_4,
    HWIF_SCAN_MAP_5, HWIF_INIT_RLIST_B15, HWIF_INIT_RLIST_F15, HWIF_REFPIC_SEG5,
    HWIF_SKIP_SEG5, HWIF_FILT_LEVEL_SEG5, HWIF_QUANT_SEG5, HWIF_TRB_PER_TRD_DM1,
    HWIF_ICOMP4_E, HWIF_ISCALE4, HWIF_ISHIFT4, HWIF_AC2_CODE8_CNT,
    HWIF_AC2_CODE7_CNT, HWIF_AC2_CODE6_CNT, HWIF_AC2_CODE5_CNT, HWIF_SCAN_MAP_6,
    HWIF_SCAN_MAP_7, HWIF_SCAN_MAP_8, HWIF_SCAN_MAP_9, HWIF_SCAN_MAP_10,
    HWIF_PARTIAL_CTB_X, HWIF_PARTIAL_CTB_Y, HWIF_PIC_WIDTH_4X4,
    HWIF_PIC_HEIGHT_4X4, HWIF_Y_STRIDE_POW2, HWIF_C_STRIDE_POW2,
    HWIF_TRB_PER_TRD_D1, HWIF_AC2_CODE12_CNT, HWIF_AC2_CODE11_CNT,
    HWIF_AC2_CODE10_CNT, HWIF_AC2_CODE9_CNT, HWIF_SCAN_MAP_11, HWIF_SCAN_MAP_12,
    HWIF_SCAN_MAP_13, HWIF_SCAN_MAP_14, HWIF_SCAN_MAP_15, HWIF_AC2_CODE16_CNT,
    HWIF_AC2_CODE15_CNT, HWIF_AC2_CODE14_CNT, HWIF_AC2_CODE13_CNT,
    HWIF_SCAN_MAP_16, HWIF_SCAN_MAP_17, HWIF_SCAN_MAP_18, HWIF_SCAN_MAP_19,
    HWIF_SCAN_MAP_20, HWIF_DC1_CODE8_CNT, HWIF_DC1_CODE7_CNT,
    HWIF_DC1_CODE6_CNT, HWIF_DC1_CODE5_CNT, HWIF_DC1_CODE4_CNT,
    HWIF_DC1_CODE3_CNT, HWIF_DC1_CODE2_CNT, HWIF_DC1_CODE1_CNT,
    HWIF_SCAN_MAP_21, HWIF_SCAN_MAP_22, HWIF_SCAN_MAP_23, HWIF_SCAN_MAP_24,
    HWIF_SCAN_MAP_25, HWIF_DC1_CODE16_CNT, HWIF_DC1_CODE15_CNT,
    HWIF_DC1_CODE14_CNT, HWIF_DC1_CODE13_CNT, HWIF_DC1_CODE12_CNT,
    HWIF_DC1_CODE11_CNT, HWIF_DC1_CODE10_CNT, HWIF_DC1_CODE9_CNT,
    HWIF_SCAN_MAP_26, HWIF_SCAN_MAP_27, HWIF_SCAN_MAP_28, HWIF_SCAN_MAP_29,
    HWIF_SCAN_MAP_30, HWIF_DC2_CODE8_CNT, HWIF_DC2_CODE7_CNT,
    HWIF_DC2_CODE6_CNT, HWIF_DC2_CODE5_CNT, HWIF_DC2_CODE4_CNT,
    HWIF_DC2_CODE3_CNT, HWIF_DC2_CODE2_CNT, HWIF_DC2_CODE1_CNT,
    HWIF_SCAN_MAP_31, HWIF_SCAN_MAP_32, HWIF_SCAN_MAP_33, HWIF_SCAN_MAP_34,
    HWIF_SCAN_MAP_35, HWIF_DC2_CODE16_CNT, HWIF_DC2_CODE15_CNT,
    HWIF_DC2_CODE14_CNT, HWIF_DC2_CODE13_CNT, HWIF_DC2_CODE12_CNT,
    HWIF_DC2_CODE11_CNT, HWIF_DC2_CODE10_CNT, HWIF_DC2_CODE9_CNT,
    HWIF_SCAN_MAP_36, HWIF_SCAN_MAP_37, HWIF_SCAN_MAP_38, HWIF_SCAN_MAP_39,
    HWIF_SCAN_MAP_40, HWIF_DC3_CODE8_CNT, HWIF_DC3_CODE7_CNT,
    HWIF_DC3_CODE6_CNT, HWIF_DC3_CODE5_CNT, HWIF_DC3_CODE4_CNT,
    HWIF_DC3_CODE3_CNT, HWIF_DC3_CODE2_CNT, HWIF_DC3_CODE1_CNT,
    HWIF_REF_INVD_CUR_1, HWIF_REF_INVD_CUR_0, HWIF_DC3_CODE16_CNT,
    HWIF_DC3_CODE15_CNT, HWIF_DC3_CODE14_CNT, HWIF_DC3_CODE13_CNT,
    HWIF_DC3_CODE12_CNT, HWIF_DC3_CODE11_CNT, HWIF_DC3_CODE10_CNT,
    HWIF_DC3_CODE9_CNT, HWIF_SCAN_MAP_41, HWIF_SCAN_MAP_42, HWIF_SCAN_MAP_43,
    HWIF_SCAN_MAP_44, HWIF_SCAN_MAP_45, HWIF_REF_INVD_CUR_3,
    HWIF_REF_INVD_CUR_2, HWIF_SCAN_MAP_46, HWIF_SCAN_MAP_47, HWIF_SCAN_MAP_48,
    HWIF_SCAN_MAP_49, HWIF_SCAN_MAP_50, HWIF_REFER1_NBR, HWIF_REFER0_NBR,
    HWIF_REF_DIST_CUR_1, HWIF_REF_DIST_CUR_0, HWIF_FILT_TYPE,
    HWIF_FILT_SHARPNESS, HWIF_FILT_MB_ADJ_0, HWIF_FILT_MB_ADJ_1,
    HWIF_FILT_MB_ADJ_2, HWIF_FILT_MB_ADJ_3, HWIF_REFER3_NBR, HWIF_REFER2_NBR,
    HWIF_SCAN_MAP_51, HWIF_SCAN_MAP_52, HWIF_SCAN_MAP_53, HWIF_SCAN_MAP_54,
    HWIF_SCAN_MAP_55, HWIF_REF_DIST_CUR_3, HWIF_REF_DIST_CUR_2,
    HWIF_REFPIC_SEG6, HWIF_SKIP_SEG6, HWIF_FILT_LEVEL_SEG6, HWIF_QUANT_SEG6,
    HWIF_REFER5_NBR, HWIF_REFER4_NBR, HWIF_SCAN_MAP_56, HWIF_SCAN_MAP_57,
    HWIF_SCAN_MAP_58, HWIF_SCAN_MAP_59, HWIF_SCAN_MAP_60, HWIF_REF_INVD_COL_1,
    HWIF_REF_INVD_COL_0, HWIF_FILT_LEVEL_0, HWIF_FILT_LEVEL_1,
    HWIF_FILT_LEVEL_2, HWIF_FILT_LEVEL_3, HWIF_REFPIC_SEG7, HWIF_SKIP_SEG7,
    HWIF_FILT_LEVEL_SEG7, HWIF_QUANT_SEG7, HWIF_REFER7_NBR, HWIF_REFER6_NBR,
    HWIF_SCAN_MAP_61, HWIF_SCAN_MAP_62, HWIF_SCAN_MAP_63, HWIF_REF_INVD_COL_3,
    HWIF_REF_INVD_COL_2, HWIF_QUANT_DELTA_0, HWIF_QUANT_DELTA_1, HWIF_QUANT_0,
    HWIF_QUANT_1, HWIF_LREF_WIDTH, HWIF_LREF_HEIGHT, HWIF_REFER9_NBR,
    HWIF_REFER8_NBR, HWIF_PRED_BC_TAP_0_3, HWIF_PRED_BC_TAP_1_0,
    HWIF_PRED_BC_TAP_1_1, HWIF_GREF_WIDTH, HWIF_GREF_HEIGHT, HWIF_REFER11_NBR,
    HWIF_REFER10_NBR, HWIF_PRED_BC_TAP_1_2, HWIF_PRED_BC_TAP_1_3,
    HWIF_PRED_BC_TAP_2_0, HWIF_AREF_WIDTH, HWIF_AREF_HEIGHT, HWIF_REFER13_NBR,
    HWIF_REFER12_NBR, HWIF_PRED_BC_TAP_2_1, HWIF_PRED_BC_TAP_2_2,
    HWIF_PRED_BC_TAP_2_3, HWIF_LREF_HOR_SCALE, HWIF_LREF_VER_SCALE,
    HWIF_REFER15_NBR, HWIF_REFER14_NBR, HWIF_PRED_BC_TAP_3_0,
    HWIF_PRED_BC_TAP_3_1, HWIF_PRED_BC_TAP_3_2, HWIF_GREF_HOR_SCALE,
    HWIF_GREF_VER_SCALE, HWIF_REFER_LTERM_E, HWIF_PRED_BC_TAP_3_3,
    HWIF_PRED_BC_TAP_4_0, HWIF_PRED_BC_TAP_4_1, HWIF_AREF_HOR_SCALE,
    HWIF_AREF_VER_SCALE, HWIF_REFER_VALID_E, HWIF_PRED_BC_TAP_4_2,
    HWIF_PRED_BC_TAP_4_3, HWIF_PRED_BC_TAP_5_0, HWIF_BINIT_RLIST_B2,
    HWIF_BINIT_RLIST_F2, HWIF_BINIT_RLIST_B1, HWIF_BINIT_RLIST_F1,
    HWIF_BINIT_RLIST_B0, HWIF_BINIT_RLIST_F0, HWIF_PRED_BC_TAP_5_1,
    HWIF_PRED_BC_TAP_5_2, HWIF_PRED_BC_TAP_5_3, HWIF_WEIGHT_QP_1,
    HWIF_REF_DELTA_COL_0, HWIF_REF_DELTA_COL_1, HWIF_REF_DELTA_COL_2,
    HWIF_REF_DELTA_COL_3, HWIF_REF_DELTA_CUR_0, HWIF_REF_DELTA_CUR_1,
    HWIF_REF_DELTA_CUR_2, HWIF_REF_DELTA_CUR_3, HWIF_LREF_Y_STRIDE,
    HWIF_LREF_C_STRIDE, HWIF_BINIT_RLIST_B5, HWIF_BINIT_RLIST_F5,
    HWIF_BINIT_RLIST_B4, HWIF_BINIT_RLIST_F4, HWIF_BINIT_RLIST_B3,
    HWIF_BINIT_RLIST_F3, HWIF_PRED_BC_TAP_6_0, HWIF_PRED_BC_TAP_6_1,
    HWIF_PRED_BC_TAP_6_2, HWIF_WEIGHT_QP_2, HWIF_WEIGHT_QP_3, HWIF_WEIGHT_QP_4,
    HWIF_WEIGHT_QP_5, HWIF_GREF_Y_STRIDE, HWIF_GREF_C_STRIDE,
    HWIF_BINIT_RLIST_B8, HWIF_BINIT_RLIST_F8, HWIF_BINIT_RLIST_B7,
    HWIF_BINIT_RLIST_F7, HWIF_BINIT_RLIST_B6, HWIF_BINIT_RLIST_F6,
    HWIF_PRED_BC_TAP_6_3, HWIF_PRED_BC_TAP_7_0, HWIF_PRED_BC_TAP_7_1,
    HWIF_DEC_AVSP_ENA, HWIF_WEIGHT_QP_E, HWIF_WEIGHT_QP_MODEL, HWIF_AVS_AEC_E,
    HWIF_NO_FWD_REF_E, HWIF_PB_FIELD_ENHANCED_E, HWIF_QP_DELTA_CB,
    HWIF_QP_DELTA_CR, HWIF_WEIGHT_QP_0, HWIF_AREF_Y_STRIDE, HWIF_AREF_C_STRIDE,
    HWIF_BINIT_RLIST_B11, HWIF_BINIT_RLIST_F11, HWIF_BINIT_RLIST_B10,
    HWIF_BINIT_RLIST_F10, HWIF_BINIT_RLIST_B9, HWIF_BINIT_RLIST_F9,
    HWIF_PRED_BC_TAP_7_2, HWIF_PRED_BC_TAP_7_3, HWIF_PRED_TAP_2_M1,
    HWIF_PRED_TAP_2_4, HWIF_PRED_TAP_4_M1, HWIF_PRED_TAP_4_4,
    HWIF_PRED_TAP_6_M1, HWIF_PRED_TAP_6_4, HWIF_BINIT_RLIST_B14,
    HWIF_BINIT_RLIST_F14, HWIF_BINIT_RLIST_B13, HWIF_BINIT_RLIST_F13,
    HWIF_BINIT_RLIST_B12, HWIF_BINIT_RLIST_F12, HWIF_QUANT_DELTA_2,
    HWIF_QUANT_DELTA_3, HWIF_QUANT_2, HWIF_QUANT_3, HWIF_CUR_POC_00,
    HWIF_CUR_POC_01, HWIF_CUR_POC_02, HWIF_CUR_POC_03, HWIF_PINIT_RLIST_F3,
    HWIF_PINIT_RLIST_F2, HWIF_PINIT_RLIST_F1, HWIF_PINIT_RLIST_F0,
    HWIF_BINIT_RLIST_B15, HWIF_BINIT_RLIST_F15, HWIF_QUANT_DELTA_4,
    HWIF_QUANT_4, HWIF_QUANT_5, HWIF_CUR_POC_04, HWIF_CUR_POC_05,
    HWIF_CUR_POC_06, HWIF_CUR_POC_07, HWIF_STARTMB_X, HWIF_STARTMB_Y,
    HWIF_ERROR_CONC_MODE, HWIF_CUR_POC_08, HWIF_CUR_POC_09, HWIF_CUR_POC_10,
    HWIF_CUR_POC_11, HWIF_PRED_BC_TAP_0_0, HWIF_PRED_BC_TAP_0_1,
    HWIF_PRED_BC_TAP_0_2, HWIF_CUR_POC_12, HWIF_CUR_POC_13, HWIF_CUR_POC_14,
    HWIF_CUR_POC_15, HWIF_APF_DISABLE, HWIF_APF_SINGLE_PU_MODE,
    HWIF_APF_THRESHOLD, HWIF_SERV_MERGE_DIS, HWIF_DEC_MULTICORE_E,
    HWIF_DEC_WRITESTAT_E, HWIF_DEC_MC_POLLMODE, HWIF_DEC_MC_POLLTIME,
    HWIF_DEC_REFER_DOUBLEBUFFER_E, HWIF_DEC_AXI_RD_ID_E, HWIF_DEC_AXI_WD_ID_E,
    HWIF_DEC_BUSWIDTH, HWIF_DEC_MAX_BURST, HWIF_GREF_SIGN_BIAS,
    HWIF_AREF_SIGN_BIAS, HWIF_FILT_REF_ADJ_0, HWIF_FILT_REF_ADJ_1,
    HWIF_FILT_REF_ADJ_2, HWIF_FILT_REF_ADJ_3, HWIF_DEC_AXI_WR_ID,
    HWIF_DEC_AXI_RD_ID, HWIF_MB_LOCATION_X, HWIF_MB_LOCATION_Y,
    HWIF_CU_LOCATION_X, HWIF_CU_LOCATION_Y, HWIF_PERF_CYCLE_COUNT,
    HWIF_DEC_OUT_YBASE_MSB, HWIF_DEC_OUT_BASE_MSB, HWIF_DEC_OUT_YBASE_LSB,
    HWIF_DEC_OUT_BASE_LSB, HWIF_DPB_ILACE_MODE, HWIF_REFER0_YBASE_MSB,
    HWIF_REFER0_BASE_MSB, HWIF_JPG_CH_OUT_BASE_MSB, HWIF_REFER0_YBASE_LSB,
    HWIF_REFER0_BASE_LSB, HWIF_REFER0_FIELD_E, HWIF_REFER0_TOPC_E,
    HWIF_JPG_CH_OUT_BASE_LSB, HWIF_REFER1_YBASE_MSB, HWIF_REFER1_BASE_MSB,
    HWIF_REFER1_YBASE_LSB, HWIF_REFER1_BASE_LSB, HWIF_REFER1_FIELD_E,
    HWIF_REFER1_TOPC_E, HWIF_REFER2_YBASE_MSB, HWIF_REFER2_BASE_MSB,
    HWIF_REFER2_YBASE_LSB, HWIF_REFER2_BASE_LSB, HWIF_REFER2_FIELD_E,
    HWIF_REFER2_TOPC_E, HWIF_REFER3_YBASE_MSB, HWIF_REFER3_BASE_MSB,
    HWIF_REFER3_YBASE_LSB, HWIF_REFER3_BASE_LSB, HWIF_REFER3_FIELD_E,
    HWIF_REFER3_TOPC_E, HWIF_REFER4_YBASE_MSB, HWIF_REFER4_BASE_MSB,
    HWIF_REFER4_YBASE_LSB, HWIF_REFER4_BASE_LSB, HWIF_REFER4_FIELD_E,
    HWIF_REFER4_TOPC_E, HWIF_REFER5_YBASE_MSB, HWIF_REFER5_BASE_MSB,
    HWIF_REFER5_YBASE_LSB, HWIF_REFER5_BASE_LSB, HWIF_REFER5_FIELD_E,
    HWIF_REFER5_TOPC_E, HWIF_REFER6_YBASE_MSB, HWIF_SEGMENT_WRITE_BASE_MSB,
    HWIF_REFER6_BASE_MSB, HWIF_VP8_DEC_CH_BASE_MSB, HWIF_REFER6_YBASE_LSB,
    HWIF_SEGMENT_WRITE_BASE_LSB, HWIF_REFER6_BASE_LSB, HWIF_REFER6_FIELD_E,
    HWIF_REFER6_TOPC_E, HWIF_VP8_DEC_CH_BASE_LSB, HWIF_VP8_STRIDE_E,
    HWIF_VP8_CH_BASE_E, HWIF_REFER7_YBASE_MSB, HWIF_SEGMENT_READ_BASE_MSB,
    HWIF_REFER7_BASE_MSB, HWIF_REFER7_YBASE_LSB, HWIF_SEGMENT_READ_BASE_LSB,
    HWIF_REFER7_BASE_LSB, HWIF_REFER7_FIELD_E, HWIF_REFER7_TOPC_E,
    HWIF_REFER8_YBASE_MSB, HWIF_REFER8_BASE_MSB, HWIF_DCT_STRM1_BASE_MSB,
    HWIF_REFER8_YBASE_LSB, HWIF_REFER8_BASE_LSB, HWIF_DCT_STRM1_BASE_LSB,
    HWIF_REFER8_FIELD_E, HWIF_REFER8_TOPC_E, HWIF_REFER9_YBASE_MSB,
    HWIF_REFER9_BASE_MSB, HWIF_DCT_STRM2_BASE_MSB, HWIF_REFER9_YBASE_LSB,
    HWIF_REFER9_BASE_LSB, HWIF_DCT_STRM2_BASE_LSB, HWIF_REFER9_FIELD_E,
    HWIF_REFER9_TOPC_E, HWIF_REFER10_YBASE_MSB, HWIF_REFER10_BASE_MSB,
    HWIF_DCT_STRM3_BASE_MSB, HWIF_REFER10_YBASE_LSB, HWIF_REFER10_BASE_LSB,
    HWIF_DCT_STRM3_BASE_LSB, HWIF_REFER10_FIELD_E, HWIF_REFER10_TOPC_E,
    HWIF_REFER11_YBASE_MSB, HWIF_REFER11_BASE_MSB, HWIF_DCT_STRM4_BASE_MSB,
    HWIF_REFER11_YBASE_LSB, HWIF_REFER11_BASE_LSB, HWIF_DCT_STRM4_BASE_LSB,
    HWIF_REFER11_FIELD_E, HWIF_REFER11_TOPC_E, HWIF_REFER12_YBASE_MSB,
    HWIF_REFER12_BASE_MSB, HWIF_DCT_STRM5_BASE_MSB, HWIF_REFER12_YBASE_LSB,
    HWIF_REFER12_BASE_LSB, HWIF_DCT_STRM5_BASE_LSB, HWIF_REFER12_FIELD_E,
    HWIF_REFER12_TOPC_E, HWIF_REFER13_YBASE_MSB, HWIF_REFER13_BASE_MSB,
    HWIF_BITPL_CTRL_BASE_MSB, HWIF_REFER13_YBASE_LSB, HWIF_REFER13_BASE_LSB,
    HWIF_REFER13_FIELD_E, HWIF_REFER13_TOPC_E, HWIF_BITPL_CTRL_BASE_LSB,
    HWIF_REFER14_YBASE_MSB, HWIF_REFER14_BASE_MSB, HWIF_DCT_STRM6_BASE_MSB,
    HWIF_REFER14_YBASE_LSB, HWIF_REFER14_BASE_LSB, HWIF_DCT_STRM6_BASE_LSB,
    HWIF_REFER14_FIELD_E, HWIF_REFER14_TOPC_E, HWIF_REFER15_YBASE_MSB,
    HWIF_REFER15_BASE_MSB, HWIF_DCT_STRM7_BASE_MSB, HWIF_REFER15_YBASE_LSB,
    HWIF_REFER15_BASE_LSB, HWIF_DCT_STRM7_BASE_LSB, HWIF_REFER15_FIELD_E,
    HWIF_REFER15_TOPC_E, HWIF_DEC_OUT_CBASE_MSB, HWIF_DEC_OUT_CBASE_LSB,
    HWIF_REFER0_CBASE_MSB, HWIF_REFER0_CBASE_LSB, HWIF_REFER1_CBASE_MSB,
    HWIF_REFER1_CBASE_LSB, HWIF_REFER2_CBASE_MSB, HWIF_REFER2_CBASE_LSB,
    HWIF_REFER3_CBASE_MSB, HWIF_REFER3_CBASE_LSB, HWIF_REFER4_CBASE_MSB,
    HWIF_REFER4_CBASE_LSB, HWIF_REFER5_CBASE_MSB, HWIF_REFER5_CBASE_LSB,
    HWIF_REFER6_CBASE_MSB, HWIF_REFER6_CBASE_LSB, HWIF_REFER7_CBASE_MSB,
    HWIF_REFER7_CBASE_LSB, HWIF_REFER8_CBASE_MSB, HWIF_REFER8_CBASE_LSB,
    HWIF_REFER9_CBASE_MSB, HWIF_REFER9_CBASE_LSB, HWIF_REFER10_CBASE_MSB,
    HWIF_REFER10_CBASE_LSB, HWIF_REFER11_CBASE_MSB, HWIF_REFER11_CBASE_LSB,
    HWIF_REFER12_CBASE_MSB, HWIF_REFER12_CBASE_LSB, HWIF_REFER13_CBASE_MSB,
    HWIF_REFER13_CBASE_LSB, HWIF_REFER14_CBASE_MSB, HWIF_REFER14_CBASE_LSB,
    HWIF_REFER15_CBASE_MSB, HWIF_REFER15_CBASE_LSB, HWIF_DEC_OUT_DBASE_MSB,
    HWIF_DIR_MV_BASE_MSB, HWIF_DEC_OUT_DBASE_LSB, HWIF_DIR_MV_BASE_LSB,
    HWIF_REFER0_DBASE_MSB, HWIF_REFER0_DBASE_LSB, HWIF_REFER1_DBASE_MSB,
    HWIF_REFER1_DBASE_LSB, HWIF_REFER2_DBASE_MSB, HWIF_REFER2_DBASE_LSB,
    HWIF_REFER3_DBASE_MSB, HWIF_REFER3_DBASE_LSB, HWIF_REFER4_DBASE_MSB,
    HWIF_REFER4_DBASE_LSB, HWIF_REFER5_DBASE_MSB, HWIF_REFER5_DBASE_LSB,
    HWIF_REFER6_DBASE_MSB, HWIF_REFER6_DBASE_LSB, HWIF_REFER7_DBASE_MSB,
    HWIF_REFER7_DBASE_LSB, HWIF_REFER8_DBASE_MSB, HWIF_REFER8_DBASE_LSB,
    HWIF_REFER9_DBASE_MSB, HWIF_REFER9_DBASE_LSB, HWIF_REFER10_DBASE_MSB,
    HWIF_REFER10_DBASE_LSB, HWIF_REFER11_DBASE_MSB, HWIF_REFER11_DBASE_LSB,
    HWIF_REFER12_DBASE_MSB, HWIF_REFER12_DBASE_LSB, HWIF_REFER13_DBASE_MSB,
    HWIF_REFER13_DBASE_LSB, HWIF_REFER14_DBASE_MSB, HWIF_REFER14_DBASE_LSB,
    HWIF_REFER15_DBASE_MSB, HWIF_REFER15_DBASE_LSB, HWIF_TILE_BASE_MSB,
    HWIF_TILE_BASE_LSB, HWIF_STREAM_BASE_MSB, HWIF_RLC_VLC_BASE_MSB,
    HWIF_STREAM_BASE_LSB, HWIF_RLC_VLC_BASE_LSB, HWIF_SCALE_LIST_BASE_MSB,
    HWIF_CTX_COUNTER_BASE_MSB, HWIF_MB_CTRL_BASE_MSB, HWIF_SCALE_LIST_BASE_LSB,
    HWIF_CTX_COUNTER_BASE_LSB, HWIF_MB_CTRL_BASE_LSB, HWIF_PROB_TAB_BASE_MSB,
    HWIF_DIFF_MV_BASE_MSB, HWIF_SEGMENT_BASE_MSB, HWIF_PROB_TAB_BASE_LSB,
    HWIF_DIFF_MV_BASE_LSB, HWIF_SEGMENT_BASE_LSB, HWIF_QTABLE_BASE_MSB,
    HWIF_QTABLE_BASE_LSB, HWIF_I4X4_OR_DC_BASE_MSB, HWIF_PJPEG_DCCB_BASE_MSB,
    HWIF_I4X4_OR_DC_BASE_LSB, HWIF_PJPEG_DCCB_BASE_LSB,
    HWIF_DEC_VERT_FILT_BASE_MSB, HWIF_PJPEG_DCCR_BASE_MSB,
    HWIF_DEC_VERT_FILT_BASE_LSB, HWIF_PJPEG_DCCR_BASE_LSB,
    HWIF_DEC_VERT_SAO_BASE_MSB, HWIF_DIR_MV_BASE2_MSB,
    HWIF_DEC_VERT_SAO_BASE_LSB, HWIF_DIR_MV_BASE2_LSB,
    HWIF_DEC_BSD_CTRL_BASE_MSB, HWIF_DEC_CH8PIX_BASE_MSB,
    HWIF_DEC_BSD_CTRL_BASE_LSB, HWIF_DEC_CH8PIX_BASE_LSB,
    HWIF_DEC_OUT_TYBASE_MSB, HWIF_DEC_OUT_TYBASE_LSB, HWIF_REFER0_TYBASE_MSB,
    HWIF_REFER0_TYBASE_LSB, HWIF_REFER1_TYBASE_MSB, HWIF_REFER1_TYBASE_LSB,
    HWIF_REFER2_TYBASE_MSB, HWIF_REFER2_TYBASE_LSB, HWIF_REFER3_TYBASE_MSB,
    HWIF_REFER3_TYBASE_LSB, HWIF_REFER4_TYBASE_MSB, HWIF_REFER4_TYBASE_LSB,
    HWIF_REFER5_TYBASE_MSB, HWIF_REFER5_TYBASE_LSB, HWIF_REFER6_TYBASE_MSB,
    HWIF_REFER6_TYBASE_LSB, HWIF_REFER7_TYBASE_MSB, HWIF_REFER7_TYBASE_LSB,
    HWIF_REFER8_TYBASE_MSB, HWIF_REFER8_TYBASE_LSB, HWIF_REFER9_TYBASE_MSB,
    HWIF_REFER9_TYBASE_LSB, HWIF_REFER10_TYBASE_MSB, HWIF_REFER10_TYBASE_LSB,
    HWIF_REFER11_TYBASE_MSB, HWIF_REFER11_TYBASE_LSB, HWIF_REFER12_TYBASE_MSB,
    HWIF_REFER12_TYBASE_LSB, HWIF_REFER13_TYBASE_MSB, HWIF_REFER13_TYBASE_LSB,
    HWIF_REFER14_TYBASE_MSB, HWIF_REFER14_TYBASE_LSB, HWIF_REFER15_TYBASE_MSB,
    HWIF_REFER15_TYBASE_LSB, HWIF_DEC_OUT_TCBASE_MSB, HWIF_DEC_OUT_TCBASE_LSB,
    HWIF_REFER0_TCBASE_MSB, HWIF_REFER0_TCBASE_LSB, HWIF_REFER1_TCBASE_MSB,
    HWIF_REFER1_TCBASE_LSB, HWIF_REFER2_TCBASE_MSB, HWIF_REFER2_TCBASE_LSB,
    HWIF_REFER3_TCBASE_MSB, HWIF_REFER3_TCBASE_LSB, HWIF_REFER4_TCBASE_MSB,
    HWIF_REFER4_TCBASE_LSB, HWIF_REFER5_TCBASE_MSB, HWIF_REFER5_TCBASE_LSB,
    HWIF_REFER6_TCBASE_MSB, HWIF_REFER6_TCBASE_LSB, HWIF_REFER7_TCBASE_MSB,
    HWIF_REFER7_TCBASE_LSB, HWIF_REFER8_TCBASE_MSB, HWIF_REFER8_TCBASE_LSB,
    HWIF_REFER9_TCBASE_MSB, HWIF_REFER9_TCBASE_LSB, HWIF_REFER10_TCBASE_MSB,
    HWIF_REFER10_TCBASE_LSB, HWIF_REFER11_TCBASE_MSB, HWIF_REFER11_TCBASE_LSB,
    HWIF_REFER12_TCBASE_MSB, HWIF_REFER12_TCBASE_LSB, HWIF_REFER13_TCBASE_MSB,
    HWIF_REFER13_TCBASE_LSB, HWIF_REFER14_TCBASE_MSB, HWIF_REFER14_TCBASE_LSB,
    HWIF_REFER15_TCBASE_MSB, HWIF_REFER15_TCBASE_LSB, HWIF_STRM_BUFFER_LEN,
    HWIF_STRM_START_OFFSET, HWIF_AXI_WR_4K_DIS, HWIF_IGNORE_SLICE_ERROR_E,
    HWIF_SKIP8X8, HWIF_QP_MAX, HWIF_QP_MIN, HWIF_QP_COUNT, HWIF_QP_SUM,
    HWIF_FWD_MVX_MAX, HWIF_FWD_MVX_MIN, HWIF_FWD_MVY_MAX, HWIF_FWD_MVY_MIN,
    HWIF_MVX_SUM_FWD, HWIF_MVX_SUM_BWD, HWIF_MVY_SUM_FWD, HWIF_MVY_SUM_BWD,
    HWIF_PB_INTER_FWD_COUNT, HWIF_PB_INTER_BWD_COUNT, HWIF_PB_INTRA_COUNT,
    HWIF_BWD_MVX_MAX, HWIF_BWD_MVX_MIN, HWIF_BWD_MVY_MAX, HWIF_BWD_MVY_MIN,
    HWIF_SIGNED_MVX_SUM_FWD, HWIF_SIGNED_MVX_SUM_BWD, HWIF_SIGNED_MVY_SUM_FWD,
    HWIF_SIGNED_MVY_SUM_BWD, HWIF_DEC_OUT_Y_STRIDE, HWIF_DEC_OUT_C_STRIDE,
    HWIF_STRM_END_OFFSET, HWIF_START_CODE_OFFSET, HWIF_LINE_CNT_INT_E,
    HWIF_PP_LINE_CNT_SEL, HWIF_LINE_CNT_STRIPE, HWIF_LINE_CNT,
    HWIF_EXT_TIMEOUT_OVERRIDE_E, HWIF_EXT_TIMEOUT_CYCLES,
    HWIF_DEC_TIMEOUT_OVERRIDE_E, HWIF_DEC_TIMEOUT_CYCLES, HWIF_PP_AXI_RD_ID_U,
    HWIF_PP_AXI_WR_ID_U, HWIF_PP_IN_BLK_SIZE_U, HWIF_PP_STATUS_U,
    HWIF_PP_OUT_TILE_E_U, HWIF_PP_OUT_MODE, HWIF_PP_CR_FIRST, HWIF_PP_OUT_E_U,
    HWIF_PP_IN_A2_SWAP_U, HWIF_PP_IN_A1_SWAP_U, HWIF_PP_OUT_SWAP_U,
    HWIF_PP_IN_SWAP_U, HWIF_PP_IN_FORMAT_U, HWIF_HOR_SCALE_MODE_U,
    HWIF_VER_SCALE_MODE_U, HWIF_PP_OUT_FORMAT_U, HWIF_SCALE_HRATIO_U,
    HWIF_RANGEMAP_Y_E_U, HWIF_RANGEMAP_C_E_U, HWIF_PP_VC1_ADV_E_U,
    HWIF_RANGEMAP_COEF_Y_U, HWIF_RANGEMAP_COEF_C_U, HWIF_SCALE_WRATIO_U,
    HWIF_WSCALE_INVRA_U, HWIF_HSCALE_INVRA_U, HWIF_PP_OUT_LU_BASE_U_MSB,
    HWIF_PP_OUT_LU_BASE_U_LSB, HWIF_PP_OUT_CH_BASE_U_MSB,
    HWIF_PP_OUT_CH_BASE_U_LSB, HWIF_PP_OUT_Y_STRIDE, HWIF_PP_OUT_C_STRIDE,
    HWIF_FLIP_MODE_U, HWIF_CROP_STARTX_U, HWIF_ROTATION_MODE_U,
    HWIF_CROP_STARTY_U, HWIF_PP_IN_WIDTH_U, HWIF_PP_IN_HEIGHT_U,
    HWIF_PP_OUT_WIDTH_U, HWIF_PP_OUT_HEIGHT_U, HWIF_PP_OUT_LU_BOT_BASE_U_MSB,
    HWIF_PP_OUT_LU_BOT_BASE_U_LSB, HWIF_PP_OUT_CH_BOT_BASE_U_MSB,
    HWIF_PP_OUT_CH_BOT_BASE_U_LSB, HWIF_PP_IN_Y_STRIDE, HWIF_PP_IN_C_STRIDE,
    HWIF_PP_IN_LU_BASE_U_MSB, HWIF_PP_IN_LU_BASE_U_LSB,
    HWIF_PP_IN_CH_BASE_U_MSB, HWIF_PP_IN_CH_BASE_U_LSB, HWIF_PP1_OUT_TILE_E_U,
    HWIF_PP1_OUT_MODE, HWIF_PP1_CR_FIRST, HWIF_PP1_OUT_E_U, HWIF_PP1_OUT_SWAP_U,
    HWIF_PP1_HOR_SCALE_MODE_U, HWIF_PP1_VER_SCALE_MODE_U, HWIF_PP1_OUT_FORMAT_U,
    HWIF_PP1_SCALE_HRATIO_U, HWIF_PP1_SCALE_WRATIO_U, HWIF_PP1_WSCALE_INVRA_U,
    HWIF_PP1_HSCALE_INVRA_U, HWIF_PP1_OUT_LU_BASE_U_MSB,
    HWIF_PP1_OUT_LU_BASE_U_LSB, HWIF_PP1_OUT_CH_BASE_U_MSB,
    HWIF_PP1_OUT_CH_BASE_U_LSB, HWIF_PP1_OUT_Y_STRIDE, HWIF_PP1_OUT_C_STRIDE,
    HWIF_PP1_FLIP_MODE_U, HWIF_PP1_CROP_STARTX_U, HWIF_PP1_ROTATION_MODE_U,
    HWIF_PP1_CROP_STARTY_U, HWIF_PP1_IN_WIDTH_U, HWIF_PP1_IN_HEIGHT_U,
    HWIF_PP1_OUT_WIDTH_U, HWIF_PP1_OUT_HEIGHT_U, HWIF_PP1_OUT_LU_BOT_BASE_U_MSB,
    HWIF_PP1_OUT_LU_BOT_BASE_U_LSB, HWIF_PP1_OUT_CH_BOT_BASE_U_MSB,
    HWIF_PP1_OUT_CH_BOT_BASE_U_LSB, HWIF_PP2_OUT_TILE_E_U, HWIF_PP2_OUT_MODE,
    HWIF_PP2_CR_FIRST, HWIF_PP2_OUT_E_U, HWIF_PP2_OUT_SWAP_U,
    HWIF_PP2_HOR_SCALE_MODE_U, HWIF_PP2_VER_SCALE_MODE_U, HWIF_PP2_OUT_FORMAT_U,
    HWIF_PP2_SCALE_HRATIO_U, HWIF_PP2_SCALE_WRATIO_U, HWIF_PP2_WSCALE_INVRA_U,
    HWIF_PP2_HSCALE_INVRA_U, HWIF_PP2_OUT_LU_BASE_U_MSB,
    HWIF_PP2_OUT_LU_BASE_U_LSB, HWIF_PP2_OUT_CH_BASE_U_MSB,
    HWIF_PP2_OUT_CH_BASE_U_LSB, HWIF_PP2_OUT_Y_STRIDE, HWIF_PP2_OUT_C_STRIDE,
    HWIF_PP2_FLIP_MODE_U, HWIF_PP2_CROP_STARTX_U, HWIF_PP2_ROTATION_MODE_U,
    HWIF_PP2_CROP_STARTY_U, HWIF_PP2_IN_WIDTH_U, HWIF_PP2_IN_HEIGHT_U,
    HWIF_PP2_OUT_WIDTH_U, HWIF_PP2_OUT_HEIGHT_U, HWIF_PP2_OUT_LU_BOT_BASE_U_MSB,
    HWIF_PP2_OUT_LU_BOT_BASE_U_LSB, HWIF_PP2_OUT_CH_BOT_BASE_U_MSB,
    HWIF_PP2_OUT_CH_BOT_BASE_U_LSB, HWIF_PP3_OUT_TILE_E_U, HWIF_PP3_OUT_MODE,
    HWIF_PP3_CR_FIRST, HWIF_PP3_OUT_E_U, HWIF_PP3_OUT_SWAP_U,
    HWIF_PP3_HOR_SCALE_MODE_U, HWIF_PP3_VER_SCALE_MODE_U, HWIF_PP3_OUT_FORMAT_U,
    HWIF_PP3_SCALE_HRATIO_U, HWIF_PP3_SCALE_WRATIO_U, HWIF_PP3_WSCALE_INVRA_U,
    HWIF_PP3_HSCALE_INVRA_U, HWIF_PP3_OUT_LU_BASE_U_MSB,
    HWIF_PP3_OUT_LU_BASE_U_LSB, HWIF_PP3_OUT_CH_BASE_U_MSB,
    HWIF_PP3_OUT_CH_BASE_U_LSB, HWIF_PP3_OUT_Y_STRIDE, HWIF_PP3_OUT_C_STRIDE,
    HWIF_PP3_FLIP_MODE_U, HWIF_PP3_CROP_STARTX_U, HWIF_PP3_ROTATION_MODE_U,
    HWIF_PP3_CROP_STARTY_U, HWIF_PP3_IN_WIDTH_U, HWIF_PP3_IN_HEIGHT_U,
    HWIF_PP3_OUT_WIDTH_U, HWIF_PP3_OUT_HEIGHT_U, HWIF_PP3_OUT_LU_BOT_BASE_U_MSB,
    HWIF_PP3_OUT_LU_BOT_BASE_U_LSB, HWIF_PP3_OUT_CH_BOT_BASE_U_MSB,
    HWIF_PP3_OUT_CH_BOT_BASE_U_LSB, HWIF_DEC_TIMEOUT_E, /* g1 */
    HWIF_DEC_STRSWAP32_E, /* g1 */
    HWIF_DEC_STRENDIAN_E, /* g1 */
    HWIF_DEC_INSWAP32_E, /* g1 */
    HWIF_DEC_OUTSWAP32_E, /* g1 */
    HWIF_DEC_DATA_DISC_E, /* g1 */
    HWIF_DEC_2CHAN_DIS, /* g1 */
    HWIF_DEC_OUT_TILED_E, /* g1 */
    HWIF_DEC_LATENCY, /* g1 */
    HWIF_DEC_IN_ENDIAN, /* g1 */
    HWIF_DEC_OUT_ENDIAN, /* g1 */
    HWIF_PRIORITY_MODE, /* g1 */
    HWIF_DEC_ADV_PRE_DIS, /* g1 */
    HWIF_DEC_SCMD_DIS, /* g1 */
    HWIF_DEC_MODE_G1V6, /* g1 */
    HWIF_RLC_MODE_E_G1V6, /* g1 */
    HWIF_PJPEG_E, /* g1 */
    HWIF_DEC_AHB_HLOCK_E, /* g1 */
    HWIF_STREAM_LEN_EXT, /* g1 */
    HWIF_VP8_CH_BASE_LSB_E, /* g1 */
    HWIF_DIR_MV_BASE_LSB2, /* g1 */
    HWIF_REFBU_E, /* g1 */
    HWIF_REFBU_THR, /* g1 */
    HWIF_REFBU_PICID, /* g1 */
    HWIF_REFBU_EVAL_E, /* g1 */
    HWIF_REFBU_FPARMOD_E, /* g1 */
    HWIF_REFBU_Y_OFFSET, /* g1 */
    HWIF_REFBU_HIT_SUM, /* g1 */
    HWIF_REFBU_INTRA_SUM, /* g1 */
    HWIF_REFBU_Y_MV_SUM, /* g1 */
    HWIF_REFBU2_BUF_E, /* g1 */
    HWIF_REFBU2_THR, /* g1 */
    HWIF_REFBU2_PICID, /* g1 */
    HWIF_REFBU_TOP_SUM, /* g1 */
    HWIF_REFBU_BOT_SUM, /* g1 */
    HWIF_DEC_CH_BASE_MSB, /* g1 */
    HWIF_CH_BASE_MSB_E, /* g1 */
    HWIF_REFER0_CH_BASE_MSB, /* g1 */
    HWIF_REFER1_CH_BASE_MSB, /* g1 */
    HWIF_REFER2_CH_BASE_MSB, /* g1 */
    HWIF_REFER3_CH_BASE_MSB, /* g1 */
    HWIF_REFER4_CH_BASE_MSB, /* g1 */
    HWIF_REFER5_CH_BASE_MSB, /* g1 */
    HWIF_REFER6_CH_BASE_MSB, /* g1 */
    HWIF_REFER7_CH_BASE_MSB, /* g1 */
    HWIF_REFER8_CH_BASE_MSB, /* g1 */
    HWIF_REFER9_CH_BASE_MSB, /* g1 */
    HWIF_REFER10_CH_BASE_MSB, /* g1 */
    HWIF_REFER11_CH_BASE_MSB, /* g1 */
    HWIF_REFER12_CH_BASE_MSB, /* g1 */
    HWIF_REFER13_CH_BASE_MSB, /* g1 */
    HWIF_REFER14_CH_BASE_MSB, /* g1 */
    HWIF_REFER15_CH_BASE_MSB, /* g1 */
    HWIF_DIR_MV_BASE_MSB2, /* g1 */
    HWIF_PP_BUS_INT, /* g1 */
    HWIF_PP_RDY_INT, /* g1 */
    HWIF_PP_IRQ, /* g1 */
    HWIF_PP_IRQ_DIS, /* g1 */
    HWIF_PP_PIPELINE_E, /* g1 */
    HWIF_PP_E, /* g1 */
    HWIF_PP_AXI_RD_ID, /* g1 */
    HWIF_PP_AXI_WR_ID, /* g1 */
    HWIF_PP_AHB_HLOCK_E, /* g1 */
    HWIF_PP_SCMD_DIS, /* g1 */
    HWIF_PP_IN_A2_ENDSEL, /* g1 */
    HWIF_PP_IN_A1_SWAP32, /* g1 */
    HWIF_PP_IN_A1_ENDIAN, /* g1 */
    HWIF_PP_IN_SWAP32_E, /* g1 */
    HWIF_PP_DATA_DISC_E, /* g1 */
    HWIF_PP_CLK_GATE_E, /* g1 */
    HWIF_PP_IN_ENDIAN, /* g1 */
    HWIF_PP_OUT_ENDIAN, /* g1 */
    HWIF_PP_OUT_SWAP32_E, /* g1 */
    HWIF_PP_MAX_BURST, /* g1 */
    HWIF_DEINT_E, /* g1 */
    HWIF_DEINT_THRESHOLD, /* g1 */
    HWIF_DEINT_BLEND_E, /* g1 */
    HWIF_DEINT_EDGE_DET, /* g1 */
    HWIF_PP_IN_LU_BASE_LSB, /* g1 */
    HWIF_PP_IN_CB_BASE_LSB, /* g1 */
    HWIF_PP_IN_CR_BASE_LSB, /* g1 */
    HWIF_PP_OUT_LU_BASE_LSB, /* g1 */
    HWIF_PP_OUT_CH_BASE_LSB, /* g1 */
    HWIF_CONTRAST_THR1, /* g1 */
    HWIF_CONTRAST_OFF2, /* g1 */
    HWIF_CONTRAST_OFF1, /* g1 */
    HWIF_PP_IN_START_CH, /* g1 */
    HWIF_PP_IN_CR_FIRST, /* g1 */
    HWIF_PP_OUT_START_CH, /* g1 */
    HWIF_PP_OUT_CR_FIRST, /* g1 */
    HWIF_COLOR_COEFFA2, /* g1 */
    HWIF_COLOR_COEFFA1, /* g1 */
    HWIF_CONTRAST_THR2, /* g1 */
    HWIF_PP_OUT_H_EXT, /* g1 */
    HWIF_COLOR_COEFFD, /* g1 */
    HWIF_COLOR_COEFFC, /* g1 */
    HWIF_COLOR_COEFFB, /* g1 */
    HWIF_PP_OUT_W_EXT, /* g1 */
    HWIF_CROP_STARTX, /* g1 */
    HWIF_ROTATION_MODE, /* g1 */
    HWIF_COLOR_COEFFF, /* g1 */
    HWIF_COLOR_COEFFE, /* g1 */
    HWIF_CROP_STARTY, /* g1 */
    HWIF_RANGEMAP_COEF_Y, /* g1 */
    HWIF_PP_IN_HEIGHT, /* g1 */
    HWIF_PP_IN_WIDTH, /* g1 */
    HWIF_PP_BOT_YIN_BASE_LSB, /* g1 */
    HWIF_PP_BOT_CIN_BASE_LSB, /* g1 */
    HWIF_RANGEMAP_Y_E, /* g1 */
    HWIF_RANGEMAP_C_E, /* g1 */
    HWIF_YCBCR_RANGE, /* g1 */
    HWIF_RGB_PIX_IN32, /* g1 */
    HWIF_RGB_R_PADD, /* g1 */
    HWIF_RGB_G_PADD, /* g1 */
    HWIF_SCALE_WRATIO, /* g1 */
    HWIF_PP_FAST_SCALE_E, /* g1 */
    HWIF_PP_IN_STRUCT, /* g1 */
    HWIF_HOR_SCALE_MODE, /* g1 */
    HWIF_VER_SCALE_MODE, /* g1 */
    HWIF_RGB_B_PADD, /* g1 */
    HWIF_SCALE_HRATIO, /* g1 */
    HWIF_WSCALE_INVRA, /* g1 */
    HWIF_HSCALE_INVRA, /* g1 */
    HWIF_R_MASK, /* g1 */
    HWIF_G_MASK, /* g1 */
    HWIF_B_MASK, /* g1 */
    HWIF_PP_IN_FORMAT, /* g1 */
    HWIF_PP_OUT_FORMAT, /* g1 */
    HWIF_PP_OUT_HEIGHT, /* g1 */
    HWIF_PP_OUT_WIDTH, /* g1 */
    HWIF_PP_OUT_TILED_E, /* g1 */
    HWIF_PP_OUT_SWAP16_E, /* g1 */
    HWIF_PP_CROP8_R_E, /* g1 */
    HWIF_PP_CROP8_D_E, /* g1 */
    HWIF_PP_IN_FORMAT_ES, /* g1 */
    HWIF_RANGEMAP_COEF_C, /* g1 */
    HWIF_MASK1_ABLEND_E, /* g1 */
    HWIF_MASK1_STARTY, /* g1 */
    HWIF_MASK1_STARTX, /* g1 */
    HWIF_MASK1_STARTX_EXT, /* g1 */
    HWIF_MASK1_STARTY_EXT, /* g1 */
    HWIF_MASK2_STARTX_EXT, /* g1 */
    HWIF_MASK2_STARTY_EXT, /* g1 */
    HWIF_MASK2_ABLEND_E, /* g1 */
    HWIF_MASK2_STARTY, /* g1 */
    HWIF_MASK2_STARTX, /* g1 */
    HWIF_EXT_ORIG_WIDTH, /* g1 */
    HWIF_MASK1_E, /* g1 */
    HWIF_MASK1_ENDY, /* g1 */
    HWIF_MASK1_ENDX, /* g1 */
    HWIF_MASK1_ENDX_EXT, /* g1 */
    HWIF_MASK1_ENDY_EXT, /* g1 */
    HWIF_MASK2_ENDX_EXT, /* g1 */
    HWIF_MASK2_ENDY_EXT, /* g1 */
    HWIF_MASK2_E, /* g1 */
    HWIF_MASK2_ENDY, /* g1 */
    HWIF_MASK2_ENDX, /* g1 */
    HWIF_RIGHT_CROSS_E, /* g1 */
    HWIF_LEFT_CROSS_E, /* g1 */
    HWIF_UP_CROSS_E, /* g1 */
    HWIF_DOWN_CROSS_E, /* g1 */
    HWIF_UP_CROSS, /* g1 */
    HWIF_DOWN_CROSS_EXT, /* g1 */
    HWIF_DOWN_CROSS, /* g1 */
    HWIF_DITHER_SELECT_R, /* g1 */
    HWIF_DITHER_SELECT_G, /* g1 */
    HWIF_DITHER_SELECT_B, /* g1 */
    HWIF_PP_TILED_MODE, /* g1 */
    HWIF_RIGHT_CROSS, /* g1 */
    HWIF_LEFT_CROSS, /* g1 */
    HWIF_PP_IN_H_EXT, /* g1 */
    HWIF_PP_IN_W_EXT, /* g1 */
    HWIF_CROP_STARTY_EXT, /* g1 */
    HWIF_CROP_STARTX_EXT, /* g1 */
    HWIF_RIGHT_CROSS_EXT, /* g1 */
    HWIF_LEFT_CROSS_EXT, /* g1 */
    HWIF_UP_CROSS_EXT, /* g1 */
    HWIF_DISPLAY_WIDTH, /* g1 */
    HWIF_ABLEND1_BASE_LSB, /* g1 */
    HWIF_ABLEND2_BASE_LSB, /* g1 */
    HWIF_ABLEND2_SCANL, /* g1 */
    HWIF_ABLEND1_SCANL, /* g1 */
    HWIF_PP_IN_LU_BASE_MSB, /* g1 */
    HWIF_PP_IN_CB_BASE_MSB, /* g1 */
    HWIF_PP_IN_CR_BASE_MSB, /* g1 */
    HWIF_PP_OUT_LU_BASE_MSB, /* g1 */
    HWIF_PP_OUT_CH_BASE_MSB, /* g1 */
    HWIF_PP_BOT_YIN_BASE_MSB, /* g1 */
    HWIF_PP_BOT_CIN_BASE_MSB, /* g1 */
    HWIF_ABLEND1_BASE_MSB, /* g1 */
    HWIF_ABLEND2_BASE_MSB, /* g1 */
    HWIF_BUFF_ID, /* g1 */
    HWIF_DEC_OUT_SWAP, /* g1 */
    HWIF_SWAP_64BIT_R, /* g1 */
    HWIF_SWAP_64BIT_W, /* g1 */
    HWIF_EXTERNAL_TIMEOUT_OVERRIDE_E, /* g1 */
    HWIF_EXTERNAL_TIMEOUT_CYCLES, /* g1 */
    HWIF_PP_OUT_PIXEL_LINES_U, /* g1 */
    HWIF_DEC_BUSBUSY, /* g2 */
    HWIF_DEC_TAB0_SWAP, /* g2 */
    HWIF_DEC_TAB1_SWAP, /* g2 */
    HWIF_DEC_TAB2_SWAP, /* g2 */
    HWIF_DEC_TAB3_SWAP, /* g2 */
    HWIF_DEC_RSCAN_SWAP, /* g2 */
    HWIF_DEC_COMP_TABLE_SWAP, /* g2 */
    HWIF_DEC_OUT_RS_E, /* g2 */
    HWIF_OUTPUT_8_BITS, /* g2 */
    HWIF_OUTPUT_FORMAT, /* g2 */
    HWIF_BUSBUSY_CYCLES, /* g2 */
    HWIF_DEC_CLK_GATE_IDLE_E, /* g2 */
    HWIF_DEC_RSY_BASE_MSB, /* g2 */
    HWIF_DEC_RSY_BASE_LSB, /* g2 */
    HWIF_DEC_RSC_BASE_MSB, /* g2 */
    HWIF_DEC_RSC_BASE_LSB, /* g2 */
    HWIF_DEC_DS_E, /* g2 */
    HWIF_DEC_DS_Y, /* g2 */
    HWIF_DEC_DS_X, /* g2 */
    HWIF_DEC_DSY_BASE_MSB, /* g2 */
    HWIF_DEC_DSY_BASE_LSB, /* g2 */
    HWIF_DEC_DSC_BASE_MSB, /* g2 */
    HWIF_DEC_DSC_BASE_LSB, /* g2 */
    HWIF_ERROR_ADDR_X, /* g2 */
    HWIF_ERROR_ADDR_Y, /* g2 */
    HWIF_ERROR_SLICE_HEADER, /* g2 */
    HWIF_ERROR_SLICE_DATA, /* g2 */