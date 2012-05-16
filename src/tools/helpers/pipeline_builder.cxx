/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "pipeline_builder.h"

#include <vistk/pipeline_util/load_pipe.h>
#include <vistk/pipeline_util/pipe_bakery.h>
#include <vistk/pipeline_util/pipe_declaration_types.h>

#include <vistk/pipeline/config.h>
#include <vistk/pipeline/pipeline.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/operations.hpp>

namespace
{

static std::string const split_str = "=";

}

pipeline_builder
::pipeline_builder()
{
}

pipeline_builder
::~pipeline_builder()
{
}

void
pipeline_builder
::load_pipeline(std::istream& istr)
{
  m_blocks = vistk::load_pipe_blocks(istr, boost::filesystem::current_path());
}

void
pipeline_builder
::load_supplement(vistk::path_t const& path)
{
  vistk::pipe_blocks const supplement = vistk::load_pipe_blocks_from_file(path);

  m_blocks.insert(m_blocks.end(), supplement.begin(), supplement.end());
}

void
pipeline_builder
::add_setting(std::string const& setting)
{
  vistk::config_pipe_block block;

  vistk::config_value_t value;

  size_t const split_pos = setting.find(split_str);

  if (split_pos == std::string::npos)
  {
    std::string const reason = "Error: The setting on the command line "
                               "\'" + setting + "\' does not contain "
                               "the \'" + split_str + "\' string which "
                               "separates the key from the value";

    throw std::runtime_error(reason);
  }

  vistk::config::key_t setting_key = setting.substr(0, split_pos);
  vistk::config::value_t setting_value = setting.substr(split_pos + split_str.size());

  vistk::config::keys_t keys;

  if (vistk::config::block_sep.size() != 1)
  {
    static std::string const reason = "Error: The block separator is longer than "
                                      "one character and does not work here";

    throw std::runtime_error(reason);
  }

  /// \bug Does not work if (vistk::config::block_sep.size() != 1).
  boost::split(keys, setting_key, boost::is_any_of(vistk::config::block_sep));

  if (keys.size() < 2)
  {
    std::string const reason = "Error: The key in the command line setting "
                               "\'" + setting + "\' does not contain "
                               "at least two keys in its keypath which is "
                               "invalid";

    throw std::runtime_error(reason);
  }

  value.key.key_path.push_back(keys.back());
  value.value = setting_value;

  keys.pop_back();

  block.key = keys;
  block.values.push_back(value);

  m_blocks.push_back(block);
}

vistk::pipeline_t
pipeline_builder
::pipeline() const
{
  return vistk::bake_pipe_blocks(m_blocks);
}

vistk::config_t
pipeline_builder
::config() const
{
  return vistk::extract_configuration(m_blocks);
}