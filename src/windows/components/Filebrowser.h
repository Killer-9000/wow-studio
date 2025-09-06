#pragma once

#include <imgui.h>

namespace Windows::Components
{
    class FileBrowser
    {
    public:
        struct File
        {
            File* parent;
            File** children;
            const char* name;
        };

        FileBrowser() { }
        ~FileBrowser() { }

        void Load()
        {
            
        }

        void Unload()
        {
            delete m_files;
            m_files = nullptr;
            m_currentFile = nullptr;
            m_fileCount = 0;

            delete m_names;
            m_names = nullptr;
            m_currentName = nullptr;
            m_nameCount = 0;
        }

        template <typename T>
        void Render(T selectCallback)
        {

        }

    private:
        void SortFiles()
        {
            
        }

        File* m_files;
        File* m_currentFile;
        size_t m_fileCount;

        char* m_names;
        char* m_currentName;
        size_t m_nameCount;
    };
}
