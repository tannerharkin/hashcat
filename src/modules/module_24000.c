/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#include "common.h"
#include "types.h"
#include "modules.h"
#include "bitops.h"
#include "convert.h"
#include "shared.h"
#include "emu_inc_cipher_des.h"

static const u32   ATTACK_EXEC    = ATTACK_EXEC_INSIDE_KERNEL;
static const u32   DGST_POS0      = 0;
static const u32   DGST_POS1      = 1;
static const u32   DGST_POS2      = 2;
static const u32   DGST_POS3      = 3;
static const u32   DGST_SIZE      = DGST_SIZE_4_4;
static const u32   HASH_CATEGORY  = HASH_CATEGORY_GENERIC;
static const char *HASH_NAME      = "Secretlounge-ng Tripcode";
static const u64   KERN_TYPE      = 24000;
static const u32   OPTI_TYPE      = OPTI_TYPE_ZERO_BYTE;
static const u64   OPTS_TYPE      = OPTS_TYPE_STOCK_MODULE
                                 | OPTS_TYPE_PT_GENERATE_LE
                                 | OPTS_TYPE_TM_KERNEL;
static const u32   SALT_TYPE      = SALT_TYPE_EMBEDDED;
static const char *ST_PASS        = "*Tp0tp8[";
static const char *ST_HASH        = "LLLLLLLLL.";

u32         module_attack_exec    (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return ATTACK_EXEC;     }
u32         module_dgst_pos0      (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return DGST_POS0;       }
u32         module_dgst_pos1      (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return DGST_POS1;       }
u32         module_dgst_pos2      (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return DGST_POS2;       }
u32         module_dgst_pos3      (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return DGST_POS3;       }
u32         module_dgst_size      (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return DGST_SIZE;       }
u32         module_hash_category  (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return HASH_CATEGORY;   }
const char *module_hash_name      (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return HASH_NAME;       }
u64         module_kern_type      (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return KERN_TYPE;       }
u32         module_opti_type      (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return OPTI_TYPE;       }
u64         module_opts_type      (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return OPTS_TYPE;       }
u32         module_salt_type      (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return SALT_TYPE;       }
const char *module_st_hash        (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return ST_HASH;         }
const char *module_st_pass        (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra) { return ST_PASS;         }

u32 module_pw_max (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra)
{
  const u32 pw_max = 8; // DES max length
  return pw_max;
}

u32 module_kernel_loops_max (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra)
{
  const u32 kernel_loops_max = 1024;
  return kernel_loops_max;
}

u32 module_kernel_loops_min (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra)
{
  const u32 kernel_loops_min = 1024;
  return kernel_loops_min;
}

int module_build_plain_postprocess (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const hashes_t *hashes, MAYBE_UNUSED const void *tmps, const u32 *src_buf, MAYBE_UNUSED const size_t src_sz, MAYBE_UNUSED const int src_len, u32 *dst_buf, MAYBE_UNUSED const size_t dst_sz)
{
  const u8 *ptr_src = (const u8 *) src_buf;
  u8 *ptr_dst = (u8 *) dst_buf;

  for (int i = 0; i < src_len; i++)
  {
    const u8 v = ptr_src[i];

    if (v & 0x80)
    {
      const u8 v2 = v & 0x7f;

      if (v2 >= 0x20)
      {
        ptr_dst[i] = v2;
      }
      else
      {
        ptr_dst[i] = v;
      }
    }
    else
    {
      ptr_dst[i] = v;
    }
  }

  return src_len;
}

int module_hash_decode (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED void *digest_buf, MAYBE_UNUSED salt_t *salt, MAYBE_UNUSED void *esalt_buf, MAYBE_UNUSED void *hook_salt_buf, MAYBE_UNUSED hashinfo_t *hash_info, const char *line_buf, MAYBE_UNUSED const int line_len)
{
  u32 *digest = (u32 *) digest_buf;

  // Skip leading '!' if present
  const char *hash_pos = line_buf;
  if (line_buf[0] == '!')
  {
    hash_pos++;
  }

  // Tripcodes are exactly 10 chars
  const int hash_len = 10;

  // Convert from base64 to bytes using Unix crypt alphabet
  u32 tmp[16] = { 0 };
  base64_decode (itoa64_to_int, (const u8 *) hash_pos, hash_len, (u8 *) tmp);

  digest[0] = tmp[0];
  digest[1] = tmp[1];
  digest[2] = 0;
  digest[3] = 0;

  // Apply initial permutation
  DES_IP (digest[0], digest[1]);

  return PARSER_OK;
}

int module_hash_encode (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const void *digest_buf, MAYBE_UNUSED const salt_t *salt, MAYBE_UNUSED const void *esalt_buf, MAYBE_UNUSED const void *hook_salt_buf, MAYBE_UNUSED const hashinfo_t *hash_info, char *line_buf, MAYBE_UNUSED const int line_size)
{
  const u32 *digest = (const u32 *) digest_buf;

  // Create temp buffer for permutation
  u32 tmp[4];
  tmp[0] = digest[0];
  tmp[1] = digest[1];
  tmp[2] = 0;
  tmp[3] = 0;

  // Apply final permutation
  DES_FP (tmp[1], tmp[0]);

  // Convert to base64
  u8 ptr_plain[12] = { 0 };
  base64_encode (int_to_itoa64, (const u8 *) tmp, 8, ptr_plain);

  // Format with leading '!' and take only 10 chars
  snprintf (line_buf, line_size, "!%.10s", ptr_plain);

  return strlen(line_buf);
}

void module_init (module_ctx_t *module_ctx)
{
  module_ctx->module_context_size             = MODULE_CONTEXT_SIZE_CURRENT;
  module_ctx->module_interface_version        = MODULE_INTERFACE_VERSION_CURRENT;

  module_ctx->module_attack_exec             = module_attack_exec;
  module_ctx->module_benchmark_esalt         = MODULE_DEFAULT;
  module_ctx->module_benchmark_hook_salt     = MODULE_DEFAULT;
  module_ctx->module_benchmark_mask          = MODULE_DEFAULT;
  module_ctx->module_benchmark_charset       = MODULE_DEFAULT;
  module_ctx->module_benchmark_salt          = MODULE_DEFAULT;
  module_ctx->module_build_plain_postprocess = module_build_plain_postprocess;
  module_ctx->module_deep_comp_kernel        = MODULE_DEFAULT;
  module_ctx->module_deprecated_notice       = MODULE_DEFAULT;
  module_ctx->module_dgst_pos0              = module_dgst_pos0;
  module_ctx->module_dgst_pos1              = module_dgst_pos1;
  module_ctx->module_dgst_pos2              = module_dgst_pos2;
  module_ctx->module_dgst_pos3              = module_dgst_pos3;
  module_ctx->module_dgst_size              = module_dgst_size;
  module_ctx->module_dictstat_disable        = MODULE_DEFAULT;
  module_ctx->module_esalt_size             = MODULE_DEFAULT;
  module_ctx->module_extra_buffer_size       = MODULE_DEFAULT;
  module_ctx->module_extra_tmp_size          = MODULE_DEFAULT;
  module_ctx->module_extra_tuningdb_block    = MODULE_DEFAULT;
  module_ctx->module_hash_category           = module_hash_category;
  module_ctx->module_hash_name               = module_hash_name;
  module_ctx->module_hash_mode               = MODULE_DEFAULT;
  module_ctx->module_hash_decode             = module_hash_decode;
  module_ctx->module_hash_encode             = module_hash_encode;
  module_ctx->module_hash_decode_postprocess = MODULE_DEFAULT;
  module_ctx->module_hash_decode_potfile     = MODULE_DEFAULT;
  module_ctx->module_hash_decode_zero_hash   = MODULE_DEFAULT;
  module_ctx->module_hash_encode_status      = MODULE_DEFAULT;
  module_ctx->module_hash_encode_potfile     = MODULE_DEFAULT;
  module_ctx->module_hashes_count_min        = MODULE_DEFAULT;
  module_ctx->module_hashes_count_max        = MODULE_DEFAULT;
  module_ctx->module_kern_type               = module_kern_type;
  module_ctx->module_kern_type_dynamic       = MODULE_DEFAULT;
  module_ctx->module_opti_type               = module_opti_type;
  module_ctx->module_opts_type               = module_opts_type;
  module_ctx->module_outfile_check_disable   = MODULE_DEFAULT;
  module_ctx->module_outfile_check_nocomp    = MODULE_DEFAULT;
  module_ctx->module_potfile_custom_check    = MODULE_DEFAULT;
  module_ctx->module_potfile_disable         = MODULE_DEFAULT;
  module_ctx->module_potfile_keep_all_hashes = MODULE_DEFAULT;
  module_ctx->module_pw_max                  = module_pw_max;
  module_ctx->module_pw_min                  = MODULE_DEFAULT;
  module_ctx->module_salt_type               = module_salt_type;
  module_ctx->module_st_hash                 = module_st_hash;
  module_ctx->module_st_pass                 = module_st_pass;
  module_ctx->module_tmp_size                = MODULE_DEFAULT;
  module_ctx->module_unstable_warning        = MODULE_DEFAULT;
  module_ctx->module_warmup_disable          = MODULE_DEFAULT;
}

bool module_unstable_warning (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra, MAYBE_UNUSED const hc_device_param_t *device_param)
{
  // Like module_1500, warn for specific problematic configurations
  if ((device_param->opencl_platform_vendor_id == VENDOR_ID_APPLE) && (device_param->opencl_device_type & CL_DEVICE_TYPE_CPU))
  {
    return true;
  }

  return false;
}

bool module_jit_cache_disable (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra, MAYBE_UNUSED const hashes_t *hashes, MAYBE_UNUSED const hc_device_param_t *device_param)
{
  return false;
}

char *module_jit_build_options (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra, MAYBE_UNUSED const hashes_t *hashes, MAYBE_UNUSED const hc_device_param_t *device_param)
{
  char *jit_build_options = NULL;

  // No special build options needed for tripcodes
  // Unlike descrypt which needs salt optimization

  return jit_build_options;
}

u32 module_salt_min (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra)
{
  const u32 salt_min = 0; // Salt is derived from password
  return salt_min;
}

u32 module_salt_max (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra)
{
  const u32 salt_max = 0; // Salt is derived from password
  return salt_max;
}

int module_hash_binary_count (MAYBE_UNUSED const hashes_t *hashes)
{
  return hashes->count;
}

int module_hash_binary_parse (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra, hashes_t *hashes)
{
  // Handle binary hash format if needed
  return PARSER_OK;
}

int module_hash_binary_save (MAYBE_UNUSED const hashes_t *hashes, MAYBE_UNUSED const u32 salt_pos, MAYBE_UNUSED const u32 digest_pos, char **buf)
{
  // Not needed for basic tripcode implementation
  return 0;
}

int module_hash_decode_postprocess (MAYBE_UNUSED const hashconfig_t *hashconfig, MAYBE_UNUSED void *digest_buf, MAYBE_UNUSED salt_t *salt, MAYBE_UNUSED void *esalt_buf, MAYBE_UNUSED void *hook_salt_buf, MAYBE_UNUSED hashinfo_t *hash_info, MAYBE_UNUSED const user_options_t *user_options, MAYBE_UNUSED const user_options_extra_t *user_options_extra)
{
  // No post-processing needed for tripcodes
  return PARSER_OK;
}

char *module_get_next_salt_tokens (char *line_buf, int line_len)
{
  // No salt tokens to parse - salt comes from password
  return line_buf;
}

void module_get_next_salt_tokens_init (char *line_buf, int line_len)
{
  // No initialization needed
}

void module_get_next_salt_tokens_done (void)
{
  // No cleanup needed
}