// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"
#include "STableRow.h"
#include "STreeView.h"
#include "DebuggerSetting.h"

class FToolBarBuilder;
class FMenuBuilder;

struct FLuaFileTreeNode
{
	FLuaFileTreeNode(bool bIsDir, FString Name, FString Path)
		: FileName(Name), BasePath(Path), IsDir(bIsDir)
	{
	}

	void FindChildren();
	void GetAllChildren(TArray<TSharedRef<FLuaFileTreeNode>>& OutArr);
	FString GetFilePath()
	{
		return BasePath / FileName;
	}
	FString FileName;
	FString BasePath;
	bool IsDir;
// 	TArray<TSharedRef<FLuaFileTreeNode>> DirChildren;
// 	TArray<TSharedRef<FLuaFileTreeNode>> FileChildren;

	TMap<FString, TSharedRef<FLuaFileTreeNode>> DirChildren;
	TMap<FString, TSharedRef<FLuaFileTreeNode>> FileChildren;
};

using FLuaFileTreeNode_Ref = TSharedRef<FLuaFileTreeNode>;
using SLuaFileTree = STreeView<FLuaFileTreeNode_Ref>;

class SLuaFileTreeWidgetItem :public STableRow<FLuaFileTreeNode_Ref>
{
public:
	
	SLATE_BEGIN_ARGS(SLuaFileTreeWidgetItem)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, FLuaFileTreeNode_Ref Node)
	{
		STableRow<FLuaFileTreeNode_Ref>::Construct(STableRow<FLuaFileTreeNode_Ref>::FArguments(), InOwnerTableView);
		ChildSlot
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SExpanderArrow, SharedThis(this))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(FText::FromString(Node->FileName))
			]
		];
	}
};

struct FCodeListNode
{
	FCodeListNode(FString _Code, int32 _Line, FString _FilePath)
		:Code(_Code), Line(_Line), HasBreakPoint(false), FilePath(_FilePath)
	{}
	FString FilePath;
	FString Code;
	int32 Line;
	bool HasBreakPoint;
};
using FCodeListNode_Ref = TSharedRef<FCodeListNode>;
using SLuaCodeList = SListView<FCodeListNode_Ref>;

class SCodeWidgetItem :public STableRow<FCodeListNode_Ref>
{
public:

	SLATE_BEGIN_ARGS(SCodeWidgetItem)
	{}
	SLATE_END_ARGS()
	TSharedPtr<FCodeListNode> CodeNode;
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, FCodeListNode_Ref Node);
	
	EVisibility BreakPointVisible() const;
	FReply HandleClickBreakPoint();
};

struct FStackListNode
{
	FStackListNode(FString _Code, int32 _Line, FString _FilePath, int32 _StackIndex)
		:Code(_Code), Line(_Line), FilePath(_FilePath), StackIndex(_StackIndex)
	{}
	FString FilePath;
	FString Code;
	int32 Line;
	int32 StackIndex;
};

using FStackListNode_Ref = TSharedRef<FStackListNode>;
using SLuaStackList = SListView<FStackListNode_Ref>;

class SLuaStackWidgetItem :public STableRow<FStackListNode_Ref>
{
public:
	TSharedPtr<FStackListNode> StackNode;
	SLATE_BEGIN_ARGS(SLuaStackWidgetItem)
	{}
	SLATE_END_ARGS()
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, FStackListNode_Ref Node);

};


using FDebuggerVarNode_Ref = TSharedRef<FDebuggerVarNode>;
using SDebuggerVarTree = STreeView<FDebuggerVarNode_Ref>;

class SDebuggerVarTreeWidgetItem :public SMultiColumnTableRow<TSharedRef<FDebuggerVarNode>>
{
public:
	static FName Col_Name;
	static FName Col_Value;
	TSharedPtr<FDebuggerVarNode> VarInfoNode;
	SLATE_BEGIN_ARGS(SDebuggerVarTreeWidgetItem)
	{}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, FDebuggerVarNode_Ref Node)
	{
		VarInfoNode = Node;
		SMultiColumnTableRow<TSharedRef<FDebuggerVarNode>>::Construct(SMultiColumnTableRow< TSharedRef<FDebuggerVarNode> >::FArguments().Padding(1), InOwnerTableView);
	}
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
};

class FLuaDebuggerModule : public IModuleInterface
{
public:
	static FLuaDebuggerModule* Ptr;
	static FLuaDebuggerModule* Get() { return Ptr; }
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();

	TArray<FLuaFileTreeNode_Ref> LuaFiles;
	TWeakPtr<SLuaFileTree> LuaFileTreePtr;

	FString NowLuaCodeFilePath;
	TArray<FCodeListNode_Ref> NowLuaCodes;
	TWeakPtr<SLuaCodeList> LuaCodeListPtr;

	TArray<FStackListNode_Ref> NowLuaStack;
	TWeakPtr<SLuaStackList> LuaStackListPtr;

	TMap<FString, TSet<int32>> BreakPoint;
	bool IsDebugRun;
	bool IsEnterDebugMode;
	bool HasBreakPoint(FString& FilePath, int32 CodeLine);
	TMap<FString, float> LastTimeFileOffset;
	void AddBreakPoint(FString& FilePath, int32 CodeLine);
	void DelBreakPoint(FString& FilePath, int32 CodeLine);
	void ToggleBreakPoint(FString& FilePath, int32 CodeLine);
	void ToggleStartDebug(const ECheckBoxState NewState);
	void BreakPointChange();
	void DebugStateChange();
	void RefreshCodeList();
	void RefreshStackList();
	static FString GetLuaSourceDir();
	static FString LuaPathToFilePath(FString LuaFilePath);
	void EnterDebug(const FString& LuaFilePath, int32 Line);
	void SetStackData(const TArray<FString>& Content, const TArray<int32>& Lines, const TArray<FString>& FilePaths, const TArray<int32>& StackIndex);
	void ShowCode(const FString& FilePath, int32 Line = 0);

	TSharedRef<ITableRow> HandleFileTreeGenerateRow(FLuaFileTreeNode_Ref InNode, const TSharedRef<STableViewBase>& OwnerTable);
	void HandleFileTreeGetChildren(FLuaFileTreeNode_Ref InNode, TArray<FLuaFileTreeNode_Ref>& OutChildren);
	void HandleFileTreeSelectionChanged(TSharedPtr<FLuaFileTreeNode>, ESelectInfo::Type);
	void HandleFileNodeExpansion(FLuaFileTreeNode_Ref InNode, bool bIsExpaned);

	TSharedRef<ITableRow> HandleCodeListGenerateRow(FCodeListNode_Ref InNode, const TSharedRef<STableViewBase>& OwnerTable);
	void HandleCodeListSelectionChanged(TSharedPtr<FCodeListNode>, ESelectInfo::Type);

	TSharedRef<ITableRow> HandleStackListGenerateRow(FStackListNode_Ref InNode, const TSharedRef<STableViewBase>& OwnerTable);
	void HandleStackListSelectionChanged(TSharedPtr<FStackListNode>, ESelectInfo::Type);

	void RefreshLuaFileData();

	FReply DebugContinue();
	void DebugTabClose(TSharedRef<SDockTab> DockTab);
private:

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};