#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/namei.h>
#include <sys/proc.h>
#include <sys/sbuf.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/vnode.h>
#include <sys/conf.h>
#include <sys/kernel.h>

#include "param.h"

static d_open_t badapple_open;
static d_close_t badapple_close;
static d_write_t badapple_write;
static d_read_t badapple_read;

static struct cdevsw badapple_cdevsw = {.d_version = D_VERSION,
                                        .d_open = badapple_open,
                                        .d_close = badapple_close,
                                        .d_read = badapple_read,
                                        .d_write = badapple_write,
                                        .d_name = "badapple"};

static struct cdev *badapple_dev;

MALLOC_DECLARE(M_BADAPPLEBUF);
MALLOC_DEFINE(M_BADAPPLEBUF, "badapplebuffer", "buffer for badapple");

struct badapple_frame_t {
  char buffer[HEIGHT][WIDTH];
};

struct badapple_frame_t *badapple_frames;

static int badapple_open(struct cdev *dev, int oflags, int devtype,
                         struct thread *td) {
  return 0;
}

static int badapple_close(struct cdev *dev, int fflag, int devtype,
                          struct thread *td) {
  return 0;
}

static int badapple_write(struct cdev *dev, struct uio *uio, int ioflag) {
  return EPERM;
}

static int badapple_read(struct cdev *dev, struct uio *uio, int ioflag) {
  size_t amt;
  off_t lframes, rframes;
  int error;

  if (uio->uio_offset % FRAMESIZE != 0)
    return (EINVAL);

  if (uio->uio_resid % FRAMESIZE != 0) {
    return (EINVAL);
  }

  lframes = uio->uio_offset / FRAMESIZE;
  rframes = MIN(((uio->uio_offset + uio->uio_resid) / FRAMESIZE),
                (FRAMERATE * VID_LENGTH_SEC)) -
            1;
  amt = (rframes - lframes + 1) * FRAMESIZE;
  error = uiomove(badapple_frames + lframes, amt, uio);

  return error;
}

static int read_file_to_buffer(const char *filename,
                               struct badapple_frame_t *frame) {
  struct sbuf *sb;
  static char buf[128];
  struct nameidata nd;
  off_t ofs;
  ssize_t resid;
  int error, flags, len;

  NDINIT(&nd, LOOKUP, FOLLOW, UIO_SYSSPACE, filename);
  flags = FREAD;
  error = vn_open(&nd, &flags, 0, NULL);
  if (error)
    return (error);

  NDFREE_PNBUF(&nd);

  ofs = 0;
  len = sizeof(buf) - 1;
  sb = sbuf_new_auto();

  while (1) {
    error =
        vn_rdwr(UIO_READ, nd.ni_vp, buf, len, ofs, UIO_SYSSPACE, IO_NODELOCKED,
                curthread->td_ucred, NOCRED, &resid, curthread);
    if (error)
      break;
    if (resid == len)
      break;
    buf[len - resid] = 0;
    sbuf_printf(sb, "%s", buf);
    ofs += len - resid;
  }

  VOP_UNLOCK(nd.ni_vp);
  vn_close(nd.ni_vp, FREAD, curthread->td_ucred, curthread);
  memcpy(frame->buffer, sbuf_data(sb), sbuf_len(sb));
  sbuf_delete(sb);
  return 0;
}

static void itoa(char *str, size_t val) {
  if (!str)
    return;

  char *iter = str;

  for (int i = 0; i < 4; ++i) {
    *iter = (val % 10) + '0';
    val /= 10;
    ++iter;
  }
  --iter;

  while (str < iter) {
    char tmp = *str;
    *str = *iter;
    *iter = tmp;
    ++str;
    --iter;
  }
}

static int build_buffer(void) {
  int err = 0;
  char current_filename[] =
      "/etc/badapple/out0001.jpg.txt";
  for (size_t i = 0; i < FRAMERATE * VID_LENGTH_SEC; ++i) {
      itoa(current_filename + 17, i + 1);
      if ((err = read_file_to_buffer(current_filename, &badapple_frames[i]))) {
          if (err == ENOENT)
              break;
          return err;
      }
  }
  return 0;
}

static int badapple_loadhandler(void) {
  int err = 0;
  badapple_frames =
      malloc(sizeof(struct badapple_frame_t) * FRAMERATE * VID_LENGTH_SEC,
             M_BADAPPLEBUF, M_WAITOK | M_ZERO);
  if ((err = build_buffer())) {
    return err;
  }
  return make_dev_p(MAKEDEV_CHECKNAME | MAKEDEV_WAITOK, &badapple_dev,
                    &badapple_cdevsw, 0, UID_ROOT, GID_WHEEL, 0600, "badapple");
}

static int lkm_event_handler(struct module *module, int event_type, void *arg) {
  int retval = 0;

  switch (event_type) {
  case MOD_LOAD:
    retval = badapple_loadhandler();
    if (retval != 0)
      break;
    uprintf("BadApple kernel module loaded");
    break;
  case MOD_UNLOAD:
    destroy_dev(badapple_dev);
    uprintf("BadApple kernel module unloaded");
    break;
  default:
    retval = EOPNOTSUPP;
    break;
  }

  return (retval);
}

static moduledata_t lkm_data = {"badapple_lkm", lkm_event_handler, NULL};

DECLARE_MODULE(badapple_lkm, lkm_data, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
