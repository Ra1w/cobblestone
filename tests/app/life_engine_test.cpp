#include "app/life_engine.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "core/entities/note.hpp"
#include "core/entities/task.hpp"
#include "mocks/mock_storage.hpp"

using ::testing::_;
using ::testing::Invoke;
using ::testing::StrictMock;

namespace {

class LifeEngineMockTest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto mock = std::make_unique<StrictMock<tests::mocks::MockStorage>>();
    mock_storage = mock.get();

    EXPECT_CALL(*mock_storage, LoadAll()).WillRepeatedly(Invoke([]() {
      return std::vector<core::interfaces::PersistenceEntry>{};
    }));

    engine = std::make_unique<app::LifeEngine>(std::move(mock));
    engine->Load();
  }

  StrictMock<tests::mocks::MockStorage>* mock_storage;
  std::unique_ptr<app::LifeEngine> engine;
};

TEST_F(LifeEngineMockTest, StrictSaveCallOnCreation) {
  EXPECT_CALL(*mock_storage, SaveAsync(_))
      .Times(1)
      .WillOnce(Invoke([](const core::entities::Entity&) {
        return tests::mocks::MakeReadyFuture();
      }));

  engine->CreateNote(core::Title("Test Note"), "Content");
  engine->WaitAllSaves();
}

TEST_F(LifeEngineMockTest, HandlesAsyncSaveFailureGracefully) {
  EXPECT_CALL(*mock_storage, SaveAsync(_))
      .Times(1)
      .WillOnce(Invoke([](const core::entities::Entity&) {
        std::promise<void> p;
        p.set_exception(
            std::make_exception_ptr(core::SystemError("Disk full")));
        return p.get_future();
      }));

  engine->CreateNote(core::Title("Bad Save Note"), "Content");

  EXPECT_NO_THROW({ engine->WaitAllSaves(); });

  auto roots = engine->GetRootEntities();
  ASSERT_EQ(roots.size(), 1);
  EXPECT_TRUE(roots[0]->IsDirty());
}

TEST_F(LifeEngineMockTest, UndoRedoOnEmptyHistoryDoesNotCrash) {
  EXPECT_FALSE(engine->CanUndo());
  EXPECT_FALSE(engine->CanRedo());

  EXPECT_NO_THROW({ engine->Undo(); });
  EXPECT_NO_THROW({ engine->Redo(); });
}

TEST_F(LifeEngineMockTest, CascadeDeleteAndUndo) {
  EXPECT_CALL(*mock_storage, SaveAsync(_)).WillOnce(Invoke([](auto&) {
    return tests::mocks::MakeReadyFuture();
  }));
  engine->CreateNote(core::Title("Parent"), "");
  core::ID parent_id = engine->GetRootEntities()[0]->GetId();

  EXPECT_CALL(*mock_storage, SaveAsync(_)).WillOnce(Invoke([](auto&) {
    return tests::mocks::MakeReadyFuture();
  }));
  engine->CreateNote(core::Title("Child"), "");
  core::ID child_id = engine->GetRootEntities()[0]->GetId() == parent_id
                          ? engine->GetRootEntities()[1]->GetId()
                          : engine->GetRootEntities()[0]->GetId();

  EXPECT_CALL(*mock_storage, SaveAsync(_)).WillRepeatedly(Invoke([](auto&) {
    return tests::mocks::MakeReadyFuture();
  }));
  engine->MoveEntity(child_id, parent_id);

  EXPECT_CALL(*mock_storage, Remove(parent_id)).Times(1);
  EXPECT_CALL(*mock_storage, Remove(child_id)).Times(1);
  engine->RemoveEntity(parent_id);

  EXPECT_EQ(engine->GetRootEntities().size(), 0);

  EXPECT_CALL(*mock_storage, SaveAsync(_))
      .Times(2)
      .WillRepeatedly(
          Invoke([](auto&) { return tests::mocks::MakeReadyFuture(); }));
  engine->Undo();

  auto roots = engine->GetRootEntities();
  ASSERT_EQ(roots.size(), 1);
  EXPECT_EQ(roots[0]->GetId(), parent_id);
  EXPECT_EQ(roots[0]->GetChildren().size(), 1);
  EXPECT_EQ(roots[0]->GetChildren()[0]->GetId(), child_id);
}

TEST_F(LifeEngineMockTest, SortingByUpdateDate) {
  EXPECT_CALL(*mock_storage, SaveAsync(_)).WillRepeatedly(Invoke([](auto&) {
    return tests::mocks::MakeReadyFuture();
  }));

  engine->CreateNote(core::Title("Old Note"), "1");
  auto roots1 = engine->GetRootEntities();
  core::ID old_id = roots1[0]->GetId();

  engine->CreateNote(core::Title("New Note"), "2");
  auto roots2 = engine->GetRootEntities();
  
  EXPECT_EQ(roots2[0]->GetMetadata().title.Str(), "New Note");
  EXPECT_EQ(roots2[1]->GetMetadata().title.Str(), "Old Note");

  engine->EditEntity(old_id, [](core::entities::Entity& e) {
    e.SetContent("Updated Content");
  });

  auto roots3 = engine->GetRootEntities();
  EXPECT_EQ(roots3[0]->GetId(), old_id);
}

TEST_F(LifeEngineMockTest, MoveEntityAndUndo) {
  EXPECT_CALL(*mock_storage, SaveAsync(_)).WillRepeatedly(Invoke([](auto&) {
    return tests::mocks::MakeReadyFuture();
  }));

  engine->CreateNote(core::Title("Root 1"), "");
  engine->CreateNote(core::Title("Root 2"), "");
  
  auto roots = engine->GetRootEntities();
  core::ID id1 = roots[0]->GetId();
  core::ID id2 = roots[1]->GetId();

  engine->MoveEntity(id2, id1);
  EXPECT_EQ(engine->GetRootEntities().size(), 1);
  EXPECT_EQ(engine->GetEntity(id2)->GetParent()->GetId(), id1);

  engine->Undo();
  EXPECT_EQ(engine->GetRootEntities().size(), 2);
  EXPECT_EQ(engine->GetEntity(id2)->GetParent(), nullptr);
}

}  // namespace
