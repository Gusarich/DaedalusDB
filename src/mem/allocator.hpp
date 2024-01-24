#pragma once

#include "mem.hpp"

namespace mem {

class PageAllocator : public std::enable_shared_from_this<PageAllocator> {

    Offset pagetable_offset_;
    size_t pages_count_;
    std::shared_ptr<mem::File> file_;
    std::shared_ptr<util::Logger> logger_;

public:
    PageAllocator() {
    }

    PageAllocator(std::shared_ptr<mem::File>& file, Offset pagetable_offset,
                  std::shared_ptr<util::Logger> logger = std::make_shared<util::EmptyLogger>())
        : pagetable_offset_(pagetable_offset), file_(file), logger_(logger) {
        pages_count_ = file_->Read<size_t>(kPagesCountOffset);
    }

    [[nodiscard]] size_t GetPagesCount() const {
        return pages_count_;
    }

    [[nodiscard]] Offset GetPagetableOffset() const {
        return pagetable_offset_;
    }

    [[nodiscard]] const std::shared_ptr<mem::File>& GetFile() const {
        return file_;
    }

    PageIndex AllocatePage() {
        if ((file_->GetSize() - pagetable_offset_) % kPageSize != 0) {
            logger_->Error("Filesize: " + std::to_string(file_->GetSize()));
            throw error::StructureError("Unaligned file");
        }
        auto new_page_offset = file_->GetSize();

        logger_->Debug("Allocating page");
        logger_->Debug("Filesize: " + std::to_string(new_page_offset));

        file_->Extend(kPageSize);
        file_->Write<Page>(Page(pages_count_++), new_page_offset);
        file_->Write<size_t>(pages_count_, kPagesCountOffset);

        logger_->Debug("Successful Allocation");

        return pages_count_ - 1;
    }

    void SwapPages(PageIndex first, PageIndex second) {
        if (first > pages_count_ || second > pages_count_) {
            throw error::BadArgument("The page index exceedes pages count: " +
                                     std::to_string(pages_count_));
        }

        logger_->Debug("Swapping pages with indecies" + std::to_string(first) + " " +
                       std::to_string(second));

        auto first_data = file_->Read<PageData>(Page(first).GetPageAddress(pagetable_offset_));
        auto second_data = file_->Read<PageData>(Page(second).GetPageAddress(pagetable_offset_));

        std::swap(first_data.page_header.index_, second_data.page_header.index_);
        std::swap(first_data.page_header.next_page_index_,
                  second_data.page_header.next_page_index_);
        std::swap(first_data.page_header.previous_page_index_,
                  second_data.page_header.previous_page_index_);

        file_->Write<PageData>(first_data,
                               first_data.page_header.GetPageAddress(pagetable_offset_));
        file_->Write<PageData>(second_data,
                               second_data.page_header.GetPageAddress(pagetable_offset_));

        logger_->Debug("Successfully swaped");
    }
};

}  // namespace mem