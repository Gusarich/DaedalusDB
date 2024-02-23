#include "test.hpp"

TEST(TypeTests, SimpleReadWrite) {
    auto file = util::MakePtr<mem::File>("test.data");
    file->Clear();
    auto name = ts::NewClass<ts::StringClass>("name");
    auto node = ts::New<ts::String>(name, "Greg");
    node->Write(file, 0);
    file->Write("Cool", 4, 0, 4);

    ASSERT_EQ("name: \"Greg\"", node->ToString());
    node->Read(file, 0);
    ASSERT_EQ("name: \"Cool\"", node->ToString());
}

TEST(TypeTests, InvalidClasses) {
    ASSERT_THROW(auto name = ts::NewClass<ts::StringClass>("name_"), error::TypeError);
    ASSERT_THROW(auto name = ts::NewClass<ts::StringClass>("n@me"), error::TypeError);
    ASSERT_THROW(auto name = ts::NewClass<ts::StringClass>("<name>"), error::TypeError);
}

TEST(TypeTests, ReadWrite) {
    auto file = util::MakePtr<mem::File>("test.data");
    file->Clear();
    auto person_class = ts::NewClass<ts::StructClass>(
        "person", ts::NewClass<ts::StringClass>("name"), ts::NewClass<ts::StringClass>("surname"),
        ts::NewClass<ts::PrimitiveClass<int>>("age"),
        ts::NewClass<ts::PrimitiveClass<bool>>("male"));

    auto node = ts::New<ts::Struct>(person_class, "Greg", "Sosnovtsev", 19, true);

    node->Write(file, 0);
    file->Write("Cool", 4, 0, 4);
    file->Write(20, 22);

    ASSERT_EQ("person: { name: \"Greg\", surname: \"Sosnovtsev\", age: 19, male: true }",
              node->ToString());
    node->Read(file, 0);
    ASSERT_EQ("person: { name: \"Cool\", surname: \"Sosnovtsev\", age: 20, male: true }",
              node->ToString());
}

TEST(TypeTests, SafeNew) {
    auto file = util::MakePtr<mem::File>("test.data");
    file->Clear();
    auto person_class = ts::NewClass<ts::StructClass>(
        "person", ts::NewClass<ts::StringClass>("name"), ts::NewClass<ts::StringClass>("surname"),
        ts::NewClass<ts::PrimitiveClass<int>>("age"),
        ts::NewClass<ts::PrimitiveClass<bool>>("male"));

    ASSERT_THROW(auto node = ts::New<ts::Struct>(person_class, "Greg", "Sosnovtsev"),
                 error::BadArgument);
}

TEST(TypeTests, DefaultNew) {
    auto file = util::MakePtr<mem::File>("test.data");
    file->Clear();
    auto person_class = ts::NewClass<ts::StructClass>(
        "person", ts::NewClass<ts::StringClass>("name"), ts::NewClass<ts::StringClass>("surname"),
        ts::NewClass<ts::PrimitiveClass<int>>("age"),
        ts::NewClass<ts::PrimitiveClass<bool>>("male"));

    auto node = ts::DefaultNew<ts::Struct>(person_class);

    ASSERT_EQ("person: { name: \"\", surname: \"\", age: 0, male: false }", node->ToString());
}

TEST(TypeTests, ReadNew) {
    auto file = util::MakePtr<mem::File>("test.data");
    file->Clear();
    auto person_class = ts::NewClass<ts::StructClass>(
        "person", ts::NewClass<ts::StringClass>("name"), ts::NewClass<ts::StringClass>("surname"),
        ts::NewClass<ts::PrimitiveClass<int>>("age"),
        ts::NewClass<ts::PrimitiveClass<bool>>("male"));

    auto node = ts::New<ts::Struct>(person_class, "Greg", "Sosnovtsev", 19, true);
    node->Write(file, 0);
    ASSERT_EQ("person: { name: \"Greg\", surname: \"Sosnovtsev\", age: 19, male: true }",
              node->ToString());
    auto new_node = ReadNew<ts::Struct>(person_class, file, 0);
    ASSERT_EQ(node->ToString(), new_node->ToString());
}

TEST(TypeTests, TypeDump) {
    auto file = util::MakePtr<mem::File>("test.data");
    file->Clear();
    // old syntax
    // auto person_class = util::MakePtr<ts::StructClass>("person");
    // person_class->AddField(ts::StringClass("name"));
    // person_class->AddField(ts::StringClass("surname"));
    // person_class->AddField(ts::PrimitiveClass<int>("age"));
    // person_class->AddField(ts::PrimitiveClass<uint64_t>("money"));

    auto person_class = ts::NewClass<ts::StructClass>(
        "person", ts::NewClass<ts::StringClass>("name"), ts::NewClass<ts::StringClass>("surname"),
        ts::NewClass<ts::PrimitiveClass<int>>("age"),
        ts::NewClass<ts::PrimitiveClass<uint64_t>>("money"));

    ts::ClassObject(person_class).Write(file, 1488);
    ASSERT_EQ(ts::ClassObject(person_class).ToString(),
              "_struct@person_<_string@name__string@surname__int@age__unsignedlong@money_>");

    ts::ClassObject read_class;
    read_class.Read(file, 1488);
    ASSERT_EQ(read_class.ToString(), ts::ClassObject(person_class).ToString());
}

TEST(TypeTests, Metadata) {
    auto person_class = ts::NewClass<ts::StructClass>(
        "person", ts::NewClass<ts::StringClass>("name"), ts::NewClass<ts::StringClass>("surname"),
        ts::NewClass<ts::PrimitiveClass<int>>("age"),
        ts::NewClass<ts::PrimitiveClass<bool>>("male"));

    ASSERT_TRUE(ts::ClassObject(person_class)
                    .Contains<ts::StringClass>(ts::NewClass<ts::StringClass>("surname")));
    ASSERT_FALSE(
        ts::ClassObject(person_class)
            .Contains<ts::PrimitiveClass<int>>(ts::NewClass<ts::PrimitiveClass<int>>("surname")));
    ASSERT_FALSE(ts::ClassObject(person_class)
                     .Contains<ts::StringClass>(ts::NewClass<ts::StringClass>("address")));
}
