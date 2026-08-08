#pragma once

#include <memory>
#include <ostream>

namespace scs {

	struct Nil;

	struct IProgram {
		virtual ~IProgram() = default;

		virtual std::shared_ptr<IProgram> clone() const = 0;

		virtual bool operator==(const Nil& nil) const {
			return false;
		}

		virtual bool operator!=(const Nil& nil) const {
			return !(*this == nil);
		}

		virtual std::ostream& Print(std::ostream& os) const = 0;
	};

	inline std::ostream& operator<< (std::ostream& os, const IProgram& prog) {
		prog.Print(os);
		return os;
	}

}
