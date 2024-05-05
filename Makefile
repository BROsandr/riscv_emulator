CC_DEBUG_FLAGS?=\
		-Wshadow                  \
		-Wformat=2                \
		-Wfloat-equal             \
		-Wconversion              \
		-Wlogical-op              \
		-Wshift-overflow=2        \
		-Wduplicated-cond         \
		-Wcast-qual               \
		-Wcast-align              \
		-D_GLIBCXX_DEBUG          \
		-D_GLIBCXX_DEBUG_PEDANTIC \
		-fno-sanitize-recover     \
		-fstack-protector         \
		-Wsign-conversion         \
		-Weffc++                  \

BUILD_TYPE?=debugoptimized
ifeq (${BUILD_TYPE},debugoptimized)
  CC_DEBUG_FLAGS+=-D_FORTIFY_SOURCE=1
else ifeq (${BUILD_TYPE},debug)
else
  $(error unknown BUILD_TYPE=${BUILD_TYPE})
endif

MESON_DEBUG_FLAGS?=$(addprefix -D,\
		cpp_debugstl=true            \
		b_ndebug=false               \
		b_sanitize=address,undefined \
)	-Dcpp_args="${CC_DEBUG_FLAGS}"

run: SAN_OPTIONS:=
run: export MALLOC_PERTURB_?=$(shell bash -c 'echo $$RANDOM')
run: export ASAN_OPTIONS:=${ASAN_OPTIONS}:debug=true:${SAN_OPTIONS}
run: export UBSAN_OPTIONS:=${UBSAN_OPTIONS}:debug=true:${SAN_OPTIONS}
run: export MSAN_OPTIONS:=${MSAN_OPTIONS}:debug=true:${SAN_OPTIONS}
run: export TSAN_OPTIONS:=${TSAN_OPTIONS}:debug=true:${SAN_OPTIONS}
run: export LSAN_OPTIONS:=${LSAN_OPTIONS}:${SAN_OPTIONS}

PROG_ARGV?=
BUILD_DIR?=${PWD}/build
MESON_EXTRA_CONFIGURE_FLAGS?=
MESON_BUILD_FLAGS?=
MESON_TEST_FLAGS?=
out?=${BUILD_DIR}
PROGRAM_NAME:=riscv_emulator

CONFIGURE_TIMESTAMP:=${BUILD_DIR}/.configure.timestamp

CONFIGURE_CMD:=meson setup "${BUILD_DIR}" --buildtype "${BUILD_TYPE}" ${MESON_DEBUG_FLAGS} ${MESON_EXTRA_CONFIGURE_FLAGS}
BUILD_CMD:=meson compile -C "${BUILD_DIR}" ${MESON_BUILD_FLAGS}
TEST_CMD:=meson test -C "${BUILD_DIR}" ${MESON_TEST_FLAGS}

default: clean configure build;

${CONFIGURE_TIMESTAMP}:
	mkdir -p "${BUILD_DIR}"
	${CONFIGURE_CMD}
	touch "${@}"

configure: clean ${CONFIGURE_TIMESTAMP};

${BUILD_DIR}/${PROGRAM_NAME}: ${CONFIGURE_TIMESTAMP}
  ifdef ANALYZE
	  scan-build ${BUILD_CMD}
  else
	  ${BUILD_CMD}
  endif

build: ${BUILD_DIR}/${PROGRAM_NAME}
	rm -f "${<}"
	$(MAKE) ${BUILD_DIR}/${PROGRAM_NAME}

run: ${BUILD_DIR}/${PROGRAM_NAME}
	${<} ${PROG_ARGV}

test: ${CONFIGURE_TIMESTAMP}
	${TEST_CMD}

install: ${BUILD_DIR}/${PROGRAM_NAME}
	mkdir -p "${out}/bin"
	cp "${BUILD_DIR}/${PROGRAM_NAME}" "${out}/bin"

clean:
	rm -rf "${BUILD_DIR}"

.PHONY: default clean build configure install run test
