@use "v1.0";

fn main() {
  const avg_customer_spending_per_year = 1000.0;

  enum BusinessCost { /* Not including expected shopping boost */
    Ticket = 100.0,
    D25 = avg_customer_spending_per_year * 0.25,
    D5 = avg_customer_spending_per_year * 0.05,
  };

  let total_costs = 0.0;

  form (CPU::cores()) (i, v in Services["UserAccountMgr"].GetUserAccountIdList()) {
    >>> f"Processing user account '{v.GetUsername()}'" <<<;

    switch (v.GetUserId() % 100) {
      case 0: {
        >>> f"User {v.GetUsername()} won the FREE ticket!" <<<;
        Services["UserAccountMgr"].PushUserDiscount(v.GetUserId(), "FREE_TICKET");

        total_costs += BusinessCost::Ticket;
      }
      case [1, 2, 3]: {
        >>> f"User {v.GetUsername()} won the 25% discount!" <<<;
        Services["UserAccountMgr"].PushUserDiscount(v.GetUserId(), "15_PERCENT_DISCOUNT_EVERYTHING");

        total_costs += BusinessCost::D25;
      }
      default: {
        >>> f"User {v.GetUsername()} won the 5% discount!" <<<;
        Services["UserAccountMgr"].PushUserDiscount(v.GetUserId(), "5_PERCENT_DISCOUNT_EVERYTHING");

        total_costs += BusinessCost::D5;
      }
    }
  }

  >>> f"Total business cost of giveaways: {total_costs}" <<<;
}