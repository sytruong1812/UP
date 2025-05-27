#include <errno.h>
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>

#include "unzip.h"
#include "unzip_vn.h"

#ifdef __cplusplus
extern "C" {
#endif

static void change_file_date(const wchar_t* filename, unsigned long dosdate, tm_unz tmu_date)
{
    FILETIME ftm, ftLocal, ftCreate, ftLastAcc, ftLastWrite;
    HANDLE hFile = CreateFileW(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    GetFileTime(hFile, &ftCreate, &ftLastAcc, &ftLastWrite);
    DosDateTimeToFileTime((WORD)(dosdate >> 16), (WORD)dosdate, &ftLocal);
    LocalFileTimeToFileTime(&ftLocal, &ftm);
    SetFileTime(hFile, &ftm, &ftLastAcc, &ftm);
    CloseHandle(hFile);
}

static int mymkdir(const wchar_t* dirname)
{
    int ret = 0;
#ifdef _WIN32
    ret = _wmkdir(dirname);
#else
    (void)dirname;
#endif
    return ret;
}

static int makedir(const wchar_t* newdir)
{
    wchar_t* buffer;
    wchar_t* p;
    unsigned long len = (unsigned long)wcslen(newdir);

    if (len == 0) return 0;

    buffer = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
    if (buffer == NULL)
    {
        return UNZ_INTERNALERROR;
    }
    wcscpy(buffer, newdir);

    if (buffer[len - 1] == L'/')
    {
        buffer[len - 1] = L'\0';
    }
    if (mymkdir(buffer) == 0)
    {
        free(buffer);
        return 1;
    }

    p = buffer + 1;
    while (1)
    {
        wchar_t hold;
        while (*p && *p != L'\\' && *p != L'/')
        {
            p++;
        }
        hold = *p;
        *p = 0;
        if ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
            free(buffer);
            return 0;
        }
        if (hold == 0)
        {
            break;
        }
        *p++ = hold;
    }
    free(buffer);
    return 1;
}

extern int ZEXPORT extractFileZip(void* uf, const char* password)
{
    wchar_t* p;
    char filename_inzip[256];
    wchar_t* filename_withoutpath;

    void* buf;
    uInt size_buf;
    int err = UNZ_OK;
    FILE* fout = NULL;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
    if (err != UNZ_OK)
    {
        return GET_CURRENT_FILE_ERROR;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf == NULL)
    {
        return ALLOCATE_MEMORY_ERROR;
    }

    p = filename_withoutpath = (wchar_t*)malloc((strlen(filename_inzip) + 1) * sizeof(wchar_t));
    mbstowcs(filename_withoutpath, filename_inzip, strlen(filename_inzip) + 1);
    while ((*p) != L'\0')
    {
        if (((*p) == L'/') || ((*p) == L'\\'))
        {
            filename_withoutpath = p + 1;
        }
        p++;
    }

    if ((*filename_withoutpath) == L'\0')
    {
        mymkdir(filename_withoutpath);
    }
    else
    {
        wchar_t* write_filename = (wchar_t*)malloc((strlen(filename_inzip) + 1) * sizeof(wchar_t));
        mbstowcs(write_filename, filename_inzip, strlen(filename_inzip) + 1);
        if ((password != NULL) && (password[0] == '\0'))
        {
            err = unzOpenCurrentFile(uf);
        }
        else
        {
            err = unzOpenCurrentFilePassword(uf, password);
        }
        if (err != UNZ_OK)
        {
            free(buf);
            free(write_filename);
            return OPEN_CURRENT_FILE_ERROR;
        }

        if (err == UNZ_OK)
        {
            FILE* ftestexist = _wfopen(write_filename, L"rb");
            if (ftestexist != NULL)
            {
                fclose(ftestexist);
            }
        }

        if (err == UNZ_OK)
        {
            fout = _wfopen(write_filename, L"wb");
            /* some zipfile don't contain directory alone before file */
            if ((fout == NULL) && (filename_withoutpath != (wchar_t*)filename_inzip))
            {
                wchar_t c = *(filename_withoutpath - 1);
                *(filename_withoutpath - 1) = L'\0';
                makedir(write_filename);
                *(filename_withoutpath - 1) = c;
                fout = _wfopen(write_filename, L"wb");
            }
            if (fout == NULL)
            {
                free(buf);
                free(write_filename);
                return OPEN_FILE_OUT_ERROR; 
            }
        }

        if (fout != NULL)
        {
            do
            {
                err = unzReadCurrentFile(uf, buf, size_buf);
                if (err < 0)
                {
                    break;
                }
                if (err > 0)
                {
                    if (fwrite(buf, (unsigned)err, 1, fout) != 1)
                    {
                        err = UNZ_ERRNO;
                        break;
                    }
                }
            } while (err > 0);
            if (fout)
            {
                fclose(fout);
            }

            if (err == 0)
            {
                change_file_date(write_filename, file_info.dosDate, file_info.tmu_date);
            }
        }
        if (err == UNZ_OK)
        {
            err = unzCloseCurrentFile(uf);
            if (err != UNZ_OK)
            {
                free(buf);
                free(write_filename);
                return CLOSE_CURRENT_FILE_ERROR; 
            }
        }
        else
        {
            unzCloseCurrentFile(uf); /* don't lose the error */
        }
        free(write_filename);
    }
    if (buf != NULL)
    {
        free(buf);
    }
    return UNZIP_VN_OK; // true
}

extern int ZEXPORT unZipBuffer(unsigned char* buf, unsigned long buf_len, const wchar_t* dest_path, const char* password)
{
    uLong i;
    unzFile uf = NULL;
    unz_global_info64 gi;

    if (_wchdir(dest_path))
    {
        return CHANGES_WORKING_DIR_ERROR;
    }
    uf = unzOpenBuffer(buf, buf_len);
    if (uf == NULL)
    {
        unzClose(uf);
        return OPEN_FILE_ZIP_ERROR;
    }
    else
    {
        int err = unzGetGlobalInfo64(uf, &gi);
        if (err != UNZ_OK)
        {
            unzClose(uf);
            return GET_GLOBAL_INFO_ERROR;
        }
        for (i = 0; i < gi.number_entry; i++)
        {
            if (!extractFileZip(uf, password))
            {
                unzClose(uf);
                return UNZIP_BUFFER_ERROR;
            }
            if ((i + 1.0) < gi.number_entry)
            {
                err = unzGoToNextFile(uf);
                if (err != UNZ_OK)
                {
                    unzClose(uf);
                    return GO_TO_NEXT_FILE_ERROR;
                }
            }
        }
    }
    unzClose(uf);
    return UNZIP_VN_OK;
}

extern int ZEXPORT unZipFile(const wchar_t* zip_path, const wchar_t* dest_path, const char* password)
{
    int error = UNZIP_VN_OK;
    unsigned char* buffer = NULL;
    unsigned long buffer_len = 0;

    FILE* zipFile = _wfopen(zip_path, L"rb");
    if (zipFile == NULL)
    {
        error = READ_ALL_FILE_ERROR;
        return error;
    }

    fseek(zipFile, 0, SEEK_END);
    buffer_len = ftell(zipFile);
    fseek(zipFile, 0, SEEK_SET);

    buffer = (unsigned char*)malloc(buffer_len);
    if (buffer == NULL)
    {
        fclose(zipFile);
        error = ALLOCATE_MEMORY_ERROR;
        return error;
    }
    unsigned long bytesRead = (unsigned long)fread(buffer, 1, buffer_len, zipFile);
    fclose(zipFile); 

    if (bytesRead != buffer_len)
    {
        free(buffer);
        error = READ_ALL_FILE_ERROR;
        return error;
    }
    if (!unZipBuffer(buffer, buffer_len, dest_path, password))
    {
        error = UNZIP_BUFFER_ERROR;
    }
    free(buffer);
    return error;
}


#ifdef __cplusplus
}
#endif