#include <stdexcept>

namespace Errors {
  struct Error : public std::runtime_error {
    using std::runtime_error::runtime_error;
  };

  class Unknown_alu_op : public Error {
    using Error::Error;
  };
}
