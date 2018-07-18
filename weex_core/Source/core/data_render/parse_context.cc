#include "core/data_render/parse_context.h"
#include "core/data_render/statistics.h"

#include <memory>

namespace weex {
namespace core {
namespace data_render {

class ParserContextImpl {
 public:
  ParserContextImpl() {}
};

ParserContext::ParserContext()
    : impl_{new ParserContextImpl()} {}

ParserContext::~ParserContext() {
  delete impl_;
}


Statistics& ParserContext::Counters() {
  return statistics_;
}

}
}
}
