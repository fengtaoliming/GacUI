#include "GuiComboControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
GuiComboBoxBase::CommandExecutor
***********************************************************************/

			GuiComboBoxBase::CommandExecutor::CommandExecutor(GuiComboBoxBase* _combo)
				:combo(_combo)
			{
			}

			GuiComboBoxBase::CommandExecutor::~CommandExecutor()
			{
			}

			void GuiComboBoxBase::CommandExecutor::SelectItem()
			{
				combo->SelectItem();
			}

/***********************************************************************
GuiComboBoxBase
***********************************************************************/

			bool GuiComboBoxBase::IsAltAvailable()
			{
				return false;
			}

			IGuiMenuService::Direction GuiComboBoxBase::GetSubMenuDirection()
			{
				return IGuiMenuService::Horizontal;
			}

			void GuiComboBoxBase::SelectItem()
			{
				styleController->OnItemSelected();
				ItemSelected.Execute(GetNotifyEventArguments());
			}

			void GuiComboBoxBase::OnBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				Size size=GetPreferredMenuClientSize();
				size.x=GetBoundsComposition()->GetBounds().Width();
				SetPreferredMenuClientSize(size);
			}

			GuiComboBoxBase::GuiComboBoxBase(IStyleController* _styleController)
				:GuiMenuButton(_styleController)
			{
				commandExecutor=new CommandExecutor(this);
				styleController=dynamic_cast<IStyleController*>(GetStyleController());
				styleController->SetCommandExecutor(commandExecutor.Obj());

				CreateSubMenu();
				SetCascadeAction(false);

				GetBoundsComposition()->BoundsChanged.AttachMethod(this, &GuiComboBoxBase::OnBoundsChanged);
			}

			GuiComboBoxBase::~GuiComboBoxBase()
			{
			}

/***********************************************************************
GuiComboBoxListControl
***********************************************************************/

			bool GuiComboBoxListControl::IsAltAvailable()
			{
				return true;
			}

			void GuiComboBoxListControl::OnActiveAlt()
			{
				GuiMenuButton::OnActiveAlt();
				GetSubMenu()->GetNativeWindow()->SetFocus();
				containedListControl->SetFocus();
			}

			void GuiComboBoxListControl::RemoveStyleController()
			{
				if (itemStyleController)
				{
					SafeDeleteComposition(itemStyleController);
					itemStyleController = nullptr;
				}
			}

			void GuiComboBoxListControl::InstallStyleController(vint itemIndex)
			{
				if (itemStyleProperty)
				{
					if (itemIndex != -1)
					{
						auto item = containedListControl->GetItemProvider()->GetBindingValue(itemIndex);
						if (!item.IsNull())
						{
							auto style = itemStyleProperty(item);
							style->SetText(GetText());
							style->SetFont(GetFont());
							style->SetVisuallyEnabled(GetVisuallyEnabled());
							if (itemStyleController)
							{
								itemStyleController->SetText(GetText());
								itemStyleController->SetFont(GetFont());
								itemStyleController->SetVisuallyEnabled(GetVisuallyEnabled());
								itemStyleController->SetAlignmentToParent(Margin(0, 0, 0, 0));
								GetContainerComposition()->AddChild(itemStyleController);
							}
						}
					}
				}
			}

			void GuiComboBoxListControl::DisplaySelectedContent(vint itemIndex)
			{
				if(itemIndex==-1)
				{
					SetText(L"");
				}
				else
				{
					WString text = containedListControl->GetItemProvider()->GetTextValue(itemIndex);
					SetText(text);
					GetSubMenu()->Hide();
				}

				RemoveStyleController();
				InstallStyleController(itemIndex);
			}

			void GuiComboBoxListControl::OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (itemStyleController)
				{
					itemStyleController->SetText(GetText());
				}
			}

			void GuiComboBoxListControl::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (itemStyleController)
				{
					itemStyleController->SetFont(GetFont());
				}
				auto args = GetNotifyEventArguments();
				OnListControlAdoptedSizeInvalidated(nullptr, args);
			}

			void GuiComboBoxListControl::OnVisuallyEnabledChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (itemStyleController)
				{
					itemStyleController->SetVisuallyEnabled(GetVisuallyEnabled());
				}
			}

			void GuiComboBoxListControl::OnListControlAdoptedSizeInvalidated(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				Size expectedSize(0, GetFont().size * 20);
				Size adoptedSize = containedListControl->GetAdoptedSize(expectedSize);

				Size clientSize = GetPreferredMenuClientSize();
				clientSize.y = adoptedSize.y + GetSubMenu()->GetClientSize().y - containedListControl->GetBoundsComposition()->GetBounds().Height();
				SetPreferredMenuClientSize(clientSize);

				if (GetSubMenuOpening())
				{
					GetSubMenu()->SetClientSize(clientSize);
				}
			}

			void GuiComboBoxListControl::OnListControlSelectionChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				DisplaySelectedContent(GetSelectedIndex());
				SelectItem();
				SelectedIndexChanged.Execute(GetNotifyEventArguments());
			}

			GuiComboBoxListControl::GuiComboBoxListControl(IStyleController* _styleController, GuiSelectableListControl* _containedListControl)
				:GuiComboBoxBase(_styleController)
				, styleController(_styleController)
				, containedListControl(_containedListControl)
			{
				styleController->SetTextVisible(true);
				TextChanged.AttachMethod(this, &GuiComboBoxListControl::OnTextChanged);
				FontChanged.AttachMethod(this, &GuiComboBoxListControl::OnFontChanged);
				VisuallyEnabledChanged.AttachMethod(this, &GuiComboBoxListControl::OnVisuallyEnabledChanged);

				containedListControl->SetMultiSelect(false);
				containedListControl->AdoptedSizeInvalidated.AttachMethod(this, &GuiComboBoxListControl::OnListControlAdoptedSizeInvalidated);
				containedListControl->SelectionChanged.AttachMethod(this, &GuiComboBoxListControl::OnListControlSelectionChanged);

				auto itemProvider = containedListControl->GetItemProvider();

				SelectedIndexChanged.SetAssociatedComposition(GetBoundsComposition());

				containedListControl->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				GetSubMenu()->GetContainerComposition()->AddChild(containedListControl->GetBoundsComposition());
				SetFont(GetFont());
			}

			GuiComboBoxListControl::~GuiComboBoxListControl()
			{
			}

			GuiSelectableListControl* GuiComboBoxListControl::GetContainedListControl()
			{
				return containedListControl;
			}

			GuiComboBoxListControl::ItemStyleProperty GuiComboBoxListControl::GetItemTemplate()
			{
				return itemStyleProperty;
			}

			void GuiComboBoxListControl::SetItemTemplate(ItemStyleProperty value)
			{
				RemoveStyleController();
				itemStyleProperty = value;
				styleController->SetTextVisible(itemStyleProperty);
				InstallStyleController(GetSelectedIndex());
				ItemTemplateChanged.Execute(GetNotifyEventArguments());
			}

			vint GuiComboBoxListControl::GetSelectedIndex()
			{
				if(containedListControl->GetSelectedItems().Count()==1)
				{
					return containedListControl->GetSelectedItems()[0];
				}
				else
				{
					return -1;
				}
			}

			void GuiComboBoxListControl::SetSelectedIndex(vint value)
			{
				containedListControl->SetSelected(value, true);
			}

			description::Value GuiComboBoxListControl::GetSelectedItem()
			{
				auto selectedIndex = GetSelectedIndex();
				if (selectedIndex != -1)
				{
					return containedListControl->GetItemProvider()->GetBindingValue(selectedIndex);
				}
				return description::Value();
			}

			GuiListControl::IItemProvider* GuiComboBoxListControl::GetItemProvider()
			{
				return containedListControl->GetItemProvider();
			}
		}
	}
}
