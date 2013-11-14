/*
 *      Copyright (C) 2013 Jean-Luc Barriere
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifndef TSDEMUXER_H
#define TSDEMUXER_H

#include "common.h"
#include "tsPacket.h"
#include "elementaryStream.h"
#include "mutex.h"

#include <map>
#include <vector>

#define FLUTS_NORMAL_TS_PACKETSIZE  188
#define FLUTS_M2TS_TS_PACKETSIZE    192
#define FLUTS_DVB_ASI_TS_PACKETSIZE 204
#define FLUTS_ATSC_TS_PACKETSIZE    208

#define AV_CONTEXT_PACKETSIZE       256

class TSDemuxer
{
public:
  virtual const unsigned char* ReadAV(uint64_t pos, size_t len) = 0;
};

enum {
  AVCONTEXT_TS_ERROR            = -3,
  AVCONTEXT_IO_ERROR            = -2,
  AVCONTEXT_TS_NOSYNC           = -1,
  AVCONTEXT_CONTINUE            = 0,
  AVCONTEXT_PROGRAM_CHANGE      = 1,
  AVCONTEXT_STREAM_PID_DATA     = 2,
  AVCONTEXT_DISCONTINUITY       = 3
};

class AVContext
{
public:
  AVContext(TSDemuxer* const demux, uint64_t pos, uint16_t channel);
  void Reset(void);

  uint16_t GetPID() const;
  PACKET_TYPE GetPIDType() const;
  uint16_t GetPIDChannel() const;
  bool HasPIDStreamData() const;
  bool HasPIDPayload() const;
  ElementaryStream* GetPIDStream();
  std::vector<ElementaryStream*> GetStreams();
  void StartStreaming(uint16_t pid);
  void StopStreaming(uint16_t pid);

  ElementaryStream* GetStream(uint16_t pid) const;
  uint16_t GetChannel(uint16_t pid) const;
  void ResetPackets();

  // TS parser
  int TSResync();
  uint64_t GoNext();
  void GoPosition(uint64_t pos);
  uint64_t GetPosition() const;
  int ProcessTSPacket();
  int ProcessTSPayload();

private:
  static STREAM_TYPE get_stream_type(uint8_t pes_type);
  static uint8_t av_rb8(const unsigned char* p);
  static uint16_t av_rb16(const unsigned char* p);
  static uint32_t av_rb32(const unsigned char* p);
  static uint64_t decode_pts(const unsigned char* p);
  void clear_pmt();
  void clear_pes(uint16_t channel);
  int parse_ts_psi();
  static ElementaryStream::STREAM_INFO parse_pes_descriptor(const unsigned char* p, size_t len, STREAM_TYPE* st);
  int parse_ts_pes();

  // Critical section
  mutable Mutex mutex;

  // AV stream owner
  TSDemuxer* m_demux;

  // Raw packet buffer
  uint64_t av_pos;
  size_t av_buf_len;
  unsigned char av_buf[AV_CONTEXT_PACKETSIZE];

  // TS Streams context
  uint16_t channel;
  std::map<uint16_t, Packet> packets;

  // Packet context
  uint16_t pid;
  bool transport_error;
  bool has_payload;
  bool payload_unit_start;
  bool discontinuity;
  const unsigned char* payload;
  size_t payload_len;
  Packet* packet;
};

#endif /* TSDEMUXER_H */

