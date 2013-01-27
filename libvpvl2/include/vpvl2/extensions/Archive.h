/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#pragma once
#ifndef VPVL2_EXTENSIONS_ARCHIVE_H_
#define VPVL2_EXTENSIONS_ARCHIVE_H_

#if defined(__APPLE__) && !defined(USE_FILE32API)
#define USE_FILE32API
#endif

#include <vpvl2/IEncoding.h>
#include <vpvl2/extensions/icu/String.h>
#include <vpvl2/extensions/minizip/ioapi.h>
#include <vpvl2/extensions/minizip/unzip.h>

#include <map>
#include <set>
#include <vector>

#include <unicode/unistr.h>
#include <unicode/regex.h>

namespace vpvl2
{
namespace extensions
{
using namespace icu;

class Archive
{
public:
    typedef std::vector<UnicodeString> EntryNames;
    enum ErrorType {
        kNone,
        kGetCurrentFileError,
        kGoToNextFileError,
        kGoToFirstFileError,
        kOpenCurrentFileError,
        kReadCurrentFileError,
        kCloseCurrentFileError,
        kMaxError
    };

    explicit Archive(IEncoding *encoding)
        : m_file(0),
          m_error(kNone),
          m_encodingRef(encoding)
    {
    }
    ~Archive() {
        close();
    }

    bool open(const IString *filename, EntryNames &entries) {
        m_file = unzOpen64(filename->toByteArray());
        if (m_file) {
            unz_file_info64 info;
            std::string filename;
            int err = unzGetGlobalInfo64(m_file, &m_header);
            if (err == UNZ_OK) {
                int nentries = m_header.number_entry;
                for (int i = 0; i < nentries; i++) {
                    err = unzGetCurrentFileInfo64(m_file, &info, 0, 0, 0, 0, 0, 0);
                    if (err == UNZ_OK && (info.compression_method == 0 || info.compression_method == Z_DEFLATED)) {
                        filename.resize(info.size_filename);
                        err = unzGetCurrentFileInfo64(m_file, 0, &filename[0], info.size_filename, 0, 0, 0, 0);
                        if (err == UNZ_OK) {
                            const uint8_t *ptr = reinterpret_cast<const uint8_t *>(filename.data());
                            IString *s = m_encodingRef->toString(ptr, filename.size(), IString::kShiftJIS);
                            entries.push_back(static_cast<const String *>(s)->value());
                            delete s;
                        }
                        else {
                            m_error = kGetCurrentFileError;
                            break;
                        }
                    }
                    else {
                        m_error = kGetCurrentFileError;
                        break;
                    }
                    if (i + 1 < nentries) {
                        err = unzGoToNextFile(m_file);
                        if (err != UNZ_OK) {
                            m_error = kGoToNextFileError;
                            break;
                        }
                    }
                }
                if (err == Z_OK) {
                    err = unzGoToFirstFile(m_file);
                    if (err != Z_OK)
                        m_error = kGoToFirstFileError;
                }
            }
            return err == Z_OK;
        }
        return false;
    }
    bool close() {
        int ret = unzClose(m_file);
        m_originalEntries.clear();
        m_filteredEntriesRef.clear();
        m_file = 0;
        return ret == Z_OK;
    }
    bool uncompress(const std::set<UnicodeString> &entries) {
        if (m_file == 0)
            return false;
        unz_file_info64 info;
        std::string filename;
        int nentries = m_header.number_entry, err = Z_OK;
        for (int i = 0; i < nentries; i++) {
            err = unzGetCurrentFileInfo64(m_file, &info, 0, 0, 0, 0, 0, 0);
            if (err == UNZ_OK && (info.compression_method == 0 || info.compression_method == Z_DEFLATED)) {
                filename.resize(info.size_filename);
                err = unzGetCurrentFileInfo64(m_file, 0, &filename[0], info.size_filename, 0, 0, 0, 0);
                if (err == UNZ_OK) {
                    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(filename.data());
                    IString *name = m_encodingRef->toString(ptr, filename.size(), IString::kShiftJIS);
                    const UnicodeString &s = static_cast<const String *>(name)->value();
                    delete name;
                    if (entries.find(s) != entries.end()) {
                        std::string &bytes = m_originalEntries[s];
                        bytes.resize(info.uncompressed_size);
                        err = unzOpenCurrentFile(m_file);
                        if (err != Z_OK) {
                            m_error = kOpenCurrentFileError;
                            break;
                        }
                        err = unzReadCurrentFile(m_file, &bytes[0], info.uncompressed_size);
                        if (err < 0) {
                            m_error = kReadCurrentFileError;
                            break;
                        }
                        err = unzCloseCurrentFile(m_file);
                        if (err != Z_OK) {
                            m_error = kCloseCurrentFileError;
                            break;
                        }
                    }
                }
                else {
                    m_error = kGetCurrentFileError;
                    break;
                }
            }
            else {
                m_error = kGetCurrentFileError;
                break;
            }
            if (i + 1 < nentries) {
                err = unzGoToNextFile(m_file);
                if (err != UNZ_OK) {
                    m_error = kGoToNextFileError;
                    break;
                }
            }
        }
        if (err == Z_OK) {
            err = unzGoToFirstFile(m_file);
            if (err != Z_OK) {
                m_error = kGoToFirstFileError;
            }
            else {
                Entries::const_iterator it = m_originalEntries.begin();
                while (it != m_originalEntries.end()) {
                    m_filteredEntriesRef.insert(std::make_pair(it->first, &it->second));
                    ++it;
                }
            }
        }
        return err == Z_OK;
    }

    void replaceFilePath(const UnicodeString &from, const UnicodeString &to) {
        EntriesRef newEntries;
        EntriesRef::const_iterator it = m_filteredEntriesRef.begin();
        UErrorCode status = U_ZERO_ERROR;
        RegexMatcher matcher("^" + from + "/", 0, status);
        while (it != m_filteredEntriesRef.end()) {
            const UnicodeString &key = it->first;
            const std::string *bytes = it->second;
            /* 一致した場合はパスを置換するが、ディレクトリ名が入っていないケースで一致しない場合はパスを追加 */
            matcher.reset(key);
            if (matcher.find()) {
                newEntries.insert(std::make_pair(matcher.replaceAll(to, status), bytes));
            }
            else {
                newEntries.insert(std::make_pair(to + key, bytes));
            }
            ++it;
        }
        m_filteredEntriesRef = newEntries;
    }
    void restoreOriginalEntries() {
        m_filteredEntriesRef.clear();
        Entries::const_iterator it = m_originalEntries.begin();
        while (it != m_originalEntries.end()) {
            m_filteredEntriesRef.insert(std::make_pair(it->first, &it->second));
            ++it;
        }
    }
    Archive::ErrorType error() const {
        return m_error;
    }
    const EntryNames entryNames() const {
        EntriesRef::const_iterator it = m_filteredEntriesRef.begin();
        EntryNames names;
        while (it != m_filteredEntriesRef.end()) {
            names.push_back(it->first);
            ++it;
        }
        return names;
    }
    const std::string *data(const UnicodeString &name) const {
        EntriesRef::const_iterator it = m_filteredEntriesRef.find(name);
        return it != m_filteredEntriesRef.end() ? it->second : 0;
    }

private:
    typedef std::map<UnicodeString, std::string> Entries;
    typedef std::map<UnicodeString, const std::string *> EntriesRef;
    unzFile m_file;
    unz_global_info64 m_header;
    ErrorType m_error;
    const IEncoding *m_encodingRef;
    Entries m_originalEntries;
    EntriesRef m_filteredEntriesRef;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Archive)
};

} /* namespace extensions */
} /* namespace vpvl2 */

#endif
