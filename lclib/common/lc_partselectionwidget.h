#pragma once

#include "lc_context.h"

class lcPartSelectionListModel;
class lcPartSelectionListView;
class lcPartSelectionWidget;

enum class lcPartCategoryType
{
	AllParts,
	PartsInUse,
	Submodels,
	Palette,
	Category,
	Count
};

enum class lcPartCategoryRole
{
	Type = Qt::UserRole,
	Index
};

struct lcPartPalette
{
	QString Name;
	std::vector<std::string> Parts;
};

class lcPartSelectionItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	lcPartSelectionItemDelegate(QObject* Parent, lcPartSelectionListModel* ListModel)
		: QStyledItemDelegate(Parent), mListModel(ListModel)
	{
	}

	void paint(QPainter* Painter, const QStyleOptionViewItem& Option, const QModelIndex& Index) const override;
	QSize sizeHint(const QStyleOptionViewItem& Option, const QModelIndex& Index) const override;

protected:
	lcPartSelectionListModel* mListModel;
};

class lcPartSelectionListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	lcPartSelectionListModel(QObject* Parent);
	~lcPartSelectionListModel();

	int rowCount(const QModelIndex& Parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& Index, int Role = Qt::DisplayRole) const override;
	QVariant headerData(int Section, Qt::Orientation Orientation, int Role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex& Index) const override;

	PieceInfo* GetPieceInfo(const QModelIndex& Index) const
	{
		return Index.isValid() ? mParts[Index.row()].first : nullptr;
	}

	PieceInfo* GetPieceInfo(int Row) const
	{
		return mParts[Row].first;
	}

	bool GetShowDecoratedParts() const
	{
		return mShowDecoratedParts;
	}

	bool GetShowPartAliases() const
	{
		return mShowPartAliases;
	}

	int GetIconSize() const
	{
		return mIconSize;
	}

	bool GetShowPartNames() const
	{
		return mShowPartNames;
	}
/*** LPub3D Mod - Part selection preview ***/
	int GetColorIndex() const
	{
		return mColorIndex;
	}
/*** LPub3D Mod end ***/
	bool IsColorLocked() const
	{
		return mColorLocked;
	}

	bool IsListMode() const
	{
		return mListMode;
	}

	void Redraw();
	void SetColorIndex(int ColorIndex);
	void ToggleColorLocked();
	void ToggleListMode();
	void SetCategory(int CategoryIndex);
	void SetModelsCategory();
	void SetPaletteCategory(int SetIndex);
	void SetCurrentModelCategory();
	void SetFilter(const QString& Filter);
	void RequestPreview(int InfoIndex);
	void SetShowDecoratedParts(bool Show);
	void SetShowPartAliases(bool Show);
	void SetIconSize(int Size);
	void SetShowPartNames(bool Show);

protected slots:
	void PartLoaded(PieceInfo* Info);

protected:
	void ClearRequests();
	void DrawPreview(int InfoIndex);

	lcPartSelectionListView* mListView;
	std::vector<std::pair<PieceInfo*, QPixmap>> mParts;
	std::vector<int> mRequestedPreviews;
	int mIconSize;
	bool mColorLocked;
	int mColorIndex;
	bool mShowPartNames;
	bool mListMode;
	bool mShowDecoratedParts;
	bool mShowPartAliases;
	QByteArray mFilter;
	std::pair<lcFramebuffer, lcFramebuffer> mRenderFramebuffer;
};

class lcPartSelectionListView : public QListView
{
	Q_OBJECT

public:
	lcPartSelectionListView(QWidget* Parent, lcPartSelectionWidget* PartSelectionWidget);

	void startDrag(Qt::DropActions SupportedActions) override;

	void SetCategory(lcPartCategoryType Type, int Index);

	PieceInfo* GetCurrentPart() const
	{
		return mListModel->GetPieceInfo(currentIndex());
	}

	lcPartSelectionListModel* GetListModel() const
	{
		return mListModel;
	}

	lcPartSelectionWidget* GetPartSelectionWidget() const
	{
		return mPartSelectionWidget;
	}

	PieceInfo* GetContextInfo() const
	{
		return mContextInfo;
	}

	void UpdateViewMode();

public slots:
	void CustomContextMenuRequested(QPoint Pos);
	void SetNoIcons();
	void SetSmallIcons();
	void SetMediumIcons();
	void SetLargeIcons();
	void SetExtraLargeIcons();
	void TogglePartNames();
	void ToggleDecoratedParts();
	void TogglePartAliases();
	void ToggleListMode();
	void ToggleFixedColor();
/*** LPub3D Mod - Part selection preview ***/
	void PreviewSelection(int InfoIndex);
/*** LPub3D Mod end ***/

protected:
/*** LPub3D Mod - Part selection preview ***/
	void mouseDoubleClickEvent(QMouseEvent *event) override;
/*** LPub3D Mod end ***/
	void SetIconSize(int Size);

	lcPartSelectionListModel* mListModel;
	lcPartSelectionWidget* mPartSelectionWidget;
	PieceInfo* mContextInfo;
	lcPartCategoryType mCategoryType;
	int mCategoryIndex;
};

class lcPartSelectionWidget : public QWidget
{
	Q_OBJECT

public:
	lcPartSelectionWidget(QWidget* Parent);

	void Redraw();
	void SetDefaultPart();
	void UpdateModels();
	void UpdateCategories();
	void LoadState(QSettings& Settings);
	void SaveState(QSettings& Settings);
	void DisableIconMode();

	void SetColorIndex(int ColorIndex)
	{
		mPartsWidget->GetListModel()->SetColorIndex(ColorIndex);
	}

	const std::vector<lcPartPalette>& GetPartPalettes() const
	{
		return mPartPalettes;
	}
/*** LPub3D Mod - set category and get current part public ***/
	void SetCategory(int CategoryIndex)
	{
	    mCategoriesWidget->setCurrentItem(mCategoriesWidget->topLevelItem(CategoryIndex + 2) /* adjust for 'All Parts' and 'In Use' initial categories*/);
	}

	PieceInfo* GetCurrentPart() const
	{
		return mPartsWidget->GetCurrentPart();
	}
/*** LPub3D Mod end ***/

public slots:
	void AddToPalette();
	void RemoveFromPalette();

protected slots:
	void DockLocationChanged(Qt::DockWidgetArea Area);
	void FilterChanged(const QString& Text);
	void FilterTriggered();
	void CategoryChanged(QTreeWidgetItem* Current, QTreeWidgetItem* Previous);
	void PartChanged(const QModelIndex& Current, const QModelIndex& Previous);
	void OptionsMenuAboutToShow();
	void EditPartPalettes();

protected:
	void LoadPartPalettes();
	void SavePartPalettes();

	void resizeEvent(QResizeEvent* Event) override;
	bool event(QEvent* Event) override;

	QTreeWidget* mCategoriesWidget;
	QLineEdit* mFilterWidget;
	QAction* mFilterAction;
	lcPartSelectionListView* mPartsWidget;
	QSplitter* mSplitter;
	std::vector<lcPartPalette> mPartPalettes;
};
